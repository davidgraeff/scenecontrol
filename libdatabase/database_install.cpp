
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

bool DatabaseInstall::installPlugindataIfMissing(const QString &pluginid, const QString &databaseImportPath)
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

		if (Database::instance()->contains(ServiceData::type(jsonData), ServiceData::id(jsonData)))
			continue;
        if (!Database::instance()->changeDocument(jsonData)) {
            qDebug() << "\tFailed to install" << pluginid << ServiceData::id(jsonData) << ", entries:" << jsonData.size();
			continue;
        }
    }

    return true;
}
