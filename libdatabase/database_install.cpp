
#include <qfileinfo.h>
#include <QDir>
#include <QDebug>

#include "database_install.h"
#include "json.h"
#include "bson.h"
#include "servicedata.h"
#include "database.h"

DatabaseInstall::DatabaseInstall(QObject* parent): QObject(parent)
{

}

bool DatabaseInstall::verifyPluginData(const QString &pluginid, const QString &databaseImportPath)
{
    if (Database::instance()->state()!=Database::ConnectedState)
        return false;

    QDir dir(databaseImportPath);
    if (!dir.cd(pluginid)) {
        qWarning() << "Database: failed to change to " << dir.absolutePath() << pluginid;
        return false;
    }

    const QStringList files = dir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
    for (int i = 0; i < files.size(); ++i) {
        QFile file(dir.absoluteFilePath(files[i]));
        file.open(QIODevice::ReadOnly);
        if (file.size() > 1024 * 10) {
            qWarning() << "\tFile to big!" << files[i] << file.size();
            continue;
        }
        bool error = false;
        QTextStream stream(&file);
        QVariantMap jsonData = JSON::parseValue(stream, error).toMap();
        file.close();
        if (error) {
            qWarning() << "\tNot a json file although json file extension!";
            continue;
        }
        if (!jsonData.contains(QLatin1String("type_"))) {
            qWarning() << "\tNo type_ in json file!";
            continue;
        }

        // Add plugin id and id before inserting into database
        jsonData[QLatin1String("plugin_")] = pluginid;
        if (!jsonData.contains(QLatin1String("_id")))
            jsonData[QLatin1String("_id")] = (QString)(QFileInfo(files[i]).completeBaseName() + pluginid);

        const mongo::BSONObj dataToSend = BJSON::toBson(jsonData);
        const mongo::BSONObj query = BSON("_id" << dataToSend.getStringField("_id"));
        const std::string dbid = "roomcontrol."+ServiceData::type(jsonData).toStdString();
        std::auto_ptr<mongo::DBClientCursor> cursor =
            Database::instance()->m_mongodb.query(dbid, query, 1);
        if (cursor.get()==0) {
            qWarning()<<"Database Install: Query failed!" << pluginid << jsonData[QLatin1String("_id")].toString();
            continue;
        }
        if (cursor->itcount()) {
            // Already in database
            continue;
        }
        Database::instance()->m_mongodb.insert(dbid, dataToSend);
        std::string lasterror = Database::instance()->m_mongodb.getLastError();
        if (lasterror.size()!=0) {
            qDebug() << "\tFailed to install" << pluginid << jsonData[QLatin1String("_id")].toString() << ", entries:" << jsonData.size();
			continue;
        }
        Database::instance()->m_mongodb.ensureIndex(dbid, mongo::fromjson("{\"plugin_\":1}"));
    }

    return true;
}
