#include "database.h"

#include <QDebug>
#include <QSocketNotifier>
#include <QTimer>
#include <QStringList>
#include <QDir>
#include <QHostInfo>
#include <QUuid>
#include <QUrl>

#include "servicedata.h"
#include "json.h"
#include "bson.h"
#include "databaselistener.h"

#define __FUNCTION__ __FUNCTION__

static Database *databaseInstance = 0;

Database::Database() :m_state(DisconnectedState), m_listener(0)
{
    m_reconnectTimer.setInterval(5000);
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, SIGNAL(timeout()), SLOT(reconnectToDatabase()));
}

Database::~Database()
{
    disconnectFromHost();
}

void Database::disconnectFromHost()
{
    if (m_listener) {
        m_listener->abort();
        delete m_listener;
        m_listener = 0;
    }
    m_reconnectTimer.stop();
}

Database *Database::instance()
{
    if (databaseInstance == 0)
        databaseInstance = new Database();

    return databaseInstance;
}


QString Database::databaseAddress() const
{
    return m_serveraddress;
}

void Database::changeState(ConnectStateEnum newstate)
{
    m_state = newstate;
    if (newstate == DisconnectedState && m_reconnectOnFailure) {
        m_state = ConnectingState;
        m_reconnectTimer.start();
    }
    emit stateChanged();
}

Database::ConnectStateEnum Database::connectToDatabase(const QString &serverHostname, bool reconnectOnFailure)
{
    m_serveraddress = serverHostname;
    //m_serveraddress = m_serveraddress.replace(QLatin1String("localhost"), QHostInfo::());
    m_reconnectOnFailure = reconnectOnFailure;
    changeState(ConnectingState);
    return reconnectToDatabase();
}

Database::ConnectStateEnum Database::reconnectToDatabase()
{
    // Read/Write mongodb timeout
    m_mongodb.setSoTimeout(5);
    try {
        m_mongodb.connect(m_serveraddress.toStdString());
        m_mongodb.createCollection("roomcontrol.listen", 50*500, true, 50, 0);
        m_state = ConnectedState;
        {
            m_listener = new DatabaseListener(m_serveraddress);
            connect(m_listener, SIGNAL(doc_changed(QString,QVariantMap)), SIGNAL(doc_changed(QString,QVariantMap)));
            connect(m_listener, SIGNAL(doc_removed(QString)), SIGNAL(doc_removed(QString)));
             m_listener->start();
        }
    } catch (mongo::UserException&e) {
        m_state = DisconnectedState;
        qWarning() << "Database: Connection failed" << m_serveraddress << QString::fromStdString(e.toString());
    }
    changeState(m_state);
    return m_state;
}

void Database::requestEvents(const QString& plugin_id, const QString& instanceid)
{
    try {
        std::auto_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query( "roomcontrol.event", BSON("plugin_" << plugin_id.toStdString() << "instanceid_" << instanceid.toStdString()) );
        while ( cursor->more() ) {
            QVariantMap jsonData = BJSON::fromBson(cursor->next());
            jsonData.remove(QLatin1String("type_"));
            if (ServiceData::collectionid(jsonData).isEmpty()) {
                qWarning() << "Database: Received event without collection:" << ServiceData::pluginid(jsonData) << ServiceData::id(jsonData);
                continue;
            }
            emit Event_add(ServiceData::id(jsonData), jsonData);
        }
    } catch (mongo::UserException&) {
        qWarning()<<"Query failed!";
    }
}

void Database::requestDataOfCollection(const QString &collecion_id)
{
    if (collecion_id.isEmpty())
        return;
    QList<QVariantMap> servicelist;
    {
        std::auto_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query("roomcontrol.event", BSON("collection_" << collecion_id.toStdString()));
        while ( cursor->more() ) {
            servicelist.append(BJSON::fromBson(cursor->next()));
        }
    }
    {
        std::auto_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query("roomcontrol.condition", BSON("collection_" << collecion_id.toStdString()));
        while ( cursor->more() ) {
            servicelist.append(BJSON::fromBson(cursor->next()));
        }
    }
    {
        std::auto_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query("roomcontrol.action", QUERY("collection_" << collecion_id.toStdString()));
        while ( cursor->more() ) {
            servicelist.append(BJSON::fromBson(cursor->next()));
        }
    }

    if (servicelist.size()) {
        emit dataOfCollection(collecion_id, servicelist);
    } else {
        qWarning() << "No actions, conditions, events found" << collecion_id;
    }
}

void Database::requestPluginConfiguration(const QString &pluginid)
{
    if (pluginid.isEmpty())
        return;
    try {
        std::auto_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query("roomcontrol.configuration", QUERY("plugin_" << pluginid.toStdString()));
        while ( cursor->more() ) {
            QVariantMap jsonData = BJSON::fromBson(cursor->next());
            jsonData.remove(QLatin1String("type_"));
            jsonData.remove(QLatin1String("plugin_"));
            emit pluginConfiguration(pluginid, jsonData);
        }
    } catch (mongo::UserException&) {
        qWarning()<<"Query failed!";
    }
}

void Database::changePluginConfiguration(const QString& pluginid, const QString& instanceid, const QByteArray& category, const QVariantMap& value)
{
    if (value.isEmpty() || instanceid.isEmpty() || pluginid.isEmpty() || category.isEmpty())
        return;

    QVariantMap jsonData = value;
    ServiceData::setPluginid(jsonData, pluginid.toAscii());
    ServiceData::setInstanceid(jsonData, instanceid);
    jsonData[QLatin1String("_id")] = pluginid.toAscii()+"_"+instanceid.toAscii()+"_"+category;
    const mongo::BSONObj dataToSend = BJSON::toBson(jsonData);
    const mongo::BSONObj query = BSON("_id" << dataToSend.getStringField("_id"));
    const std::string dbid = "roomcontrol.configuration";
    m_mongodb.update(dbid, query, dataToSend, true, false);
    m_mongodb.ensureIndex(dbid, mongo::fromjson("{\"plugin_\":1}"));
    // update listen collection
    mongo::BSONObjBuilder b;
    b.appendTimestamp("_id");
    b.append("op", "u");
    b.append("o", dataToSend);
    m_mongodb.insert("roomcontrol.listen", b.done());
}

void Database::requestSchemas()
{
    std::auto_ptr<mongo::DBClientCursor> cursor =
        m_mongodb.query("roomcontrol.schema", mongo::BSONObj());
    while ( cursor->more() ) {
        QVariantMap jsonData = BJSON::fromBson(cursor->next());
        emit doc_changed(ServiceData::id(jsonData), jsonData);
    }
}

void Database::requestCollections()
{
    std::auto_ptr<mongo::DBClientCursor> cursor =
        m_mongodb.query("roomcontrol.collection", mongo::BSONObj());
    while ( cursor->more() ) {
        QVariantMap jsonData = BJSON::fromBson(cursor->next());
        emit doc_changed(ServiceData::id(jsonData), jsonData);
    }
}

QVariantMap Database::checkTypes(const QVariantMap& data, const QVariantMap& types)
{
    QVariantMap result;
    // convert types if neccessary (if qml send data for example)
    QVariantMap::const_iterator i = data.begin();
    for (;i!=data.end();++i) {
        const QByteArray targettype = types.value(i.key()).toByteArray();
        if (targettype.isEmpty()) {
            result[i.key()] = i.value();
            continue;
        }
        QVariant element = i.value();
        if (element.type()!=QVariant::nameToType(targettype) && !element.convert(QVariant::nameToType(targettype))) {
            qWarning()<<"checkTypes: Conversion failed" << i.key() << "orig:" << i.value().typeName() << "dest:" << targettype;
            continue;
        }
        result[i.key()] = element;
    }

    if (result.size() < data.size())
        return QVariantMap();
    return result;
}

void Database::removeDocument(const QString &type, const QString &id)
{
    if (type.isEmpty()||id.isEmpty())
        return;

    m_mongodb.remove("roomcontrol."+type.toStdString(), BSON("_id" << id.toStdString()));
    if (type == QLatin1String("collection")) {
        // if it is a collection, remove all actions, conditions, events belonging to it
        // Do it in a per-element way to update the listen collection after every operation
        std::auto_ptr<mongo::DBClientCursor> cursor;
        cursor = m_mongodb.query("roomcontrol.action", BSON("collection_" << id.toStdString()));
        while ( cursor->more() ) {
            std::string elementid = cursor->next().getStringField("_id");
            removeDocument(QLatin1String("action"), QString::fromStdString(elementid));
        }
        cursor = m_mongodb.query("roomcontrol.event", BSON("collection_" << id.toStdString()));
        while ( cursor->more() ) {
            std::string elementid = cursor->next().getStringField("_id");
            removeDocument(QLatin1String("event"), QString::fromStdString(elementid));
        }
        cursor = m_mongodb.query("roomcontrol.condition", BSON("collection_" << id.toStdString()));
        while ( cursor->more() ) {
            std::string elementid = cursor->next().getStringField("_id");
            removeDocument(QLatin1String("condition"), QString::fromStdString(elementid));
        }
    }

    // update listen collection
    mongo::BSONObjBuilder b;
    b.appendTimestamp("_id");
    b.append("op", "d");
    b.append("o", BSON("_id" << id.toStdString()));
    m_mongodb.insert("roomcontrol.listen", b.done());

}

bool Database::changeDocument(const QVariantMap& data, bool insertWithNewID, const QVariantMap& types)
{
    if (!data.contains(QLatin1String("type_"))) {
        qWarning() << "changeDocument: can not add/change document without type_";
        return false;
    }

    QVariantMap jsonData = data;
    if (!jsonData.contains(QLatin1String("_id"))) {
        if (insertWithNewID)
            jsonData[QLatin1String("_id")] = QUuid::createUuid().toString().
                                             replace(QLatin1String("{"),QString()).
                                             replace(QLatin1String("}"),QString()).
                                             replace(QLatin1String("-"),QString());
        else {
            qWarning() << "changeDocument: can not change document without _id";
            return false;
        }
    }

    if (types.size()) {
        jsonData = checkTypes(jsonData, types);
    }

    const QString docid = jsonData[QLatin1String("_id")].toString();
    const mongo::BSONObj dataToSend = BJSON::toBson(jsonData);
    const mongo::BSONObj query = BSON("_id" << docid.toStdString());
    const std::string dbid = "roomcontrol."+ServiceData::type(jsonData).toStdString();
    m_mongodb.update(dbid, query, dataToSend, true, false);

    std::string lasterror = m_mongodb.getLastError();
    if (lasterror.size()!=0) {
        return false;
    }

    // update listen collection
    mongo::BSONObjBuilder b;
    b.appendTimestamp("_id");
    b.append("op", "u");
    b.append("o", dataToSend);
    m_mongodb.insert("roomcontrol.listen", b.done());
	
    if (jsonData.contains(QLatin1String("plugin_")) && jsonData.contains(QLatin1String("instanceid_")))
        m_mongodb.ensureIndex(dbid, mongo::fromjson("{\"plugin_\":1,\"instanceid_\":1}"));
    else if (jsonData.contains(QLatin1String("plugin_")))
        m_mongodb.ensureIndex(dbid, mongo::fromjson("{\"plugin_\":1}"));
    if (jsonData.contains(QLatin1String("collection_")))
        m_mongodb.ensureIndex(dbid, mongo::fromjson("{\"collection_\":1}"));

    emit doc_changed(docid, jsonData);
    return true;
}

bool Database::contains(const QString& type, const QString& id)
{
    const mongo::BSONObj query = BSON("_id" << id.toStdString());
    const std::string dbid = "roomcontrol."+type.toStdString();
    std::auto_ptr<mongo::DBClientCursor> cursor =
        m_mongodb.query(dbid, query, 1);
    if (cursor.get()==0) {
        qWarning()<<"Database: Query failed!" << type << id;
        return false;
    }
    return cursor->itcount();
}

void Database::exportAsJSON(const QString& path)
{
    std::list<std::string> collections = m_mongodb.getCollectionNames("roomcontrol");
    std::list<std::string>::const_iterator i = collections.begin();
    for (;i!= collections.end();++i) {
        const QString collectionname = QString::fromStdString(*i);
        std::auto_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query(collectionname.toStdString(), mongo::BSONObj());
        while ( cursor->more() ) {
            QVariantMap jsonData = BJSON::fromBson(cursor->next());
            const QString id = jsonData.value(QLatin1String("_id")).toString();
            QDir dir(path);
            if (jsonData.contains(QLatin1String("plugin_"))) {
                const QString pluginid = jsonData.value(QLatin1String("plugin_")).toString();
                if (!dir.cd(pluginid) && (!dir.mkdir(pluginid) || !dir.cd(pluginid))) {
                    qWarning() << "Database: Failed to create subdir" << pluginid << dir;
                    continue;
                }
            }
            if (!dir.cd(collectionname) && (!dir.mkdir(collectionname) || !dir.cd(collectionname))) {
                qWarning() << "Database: Failed to create subdir" << collectionname << dir;
                continue;
            }

            QFile f(dir.absoluteFilePath(id + QLatin1String(".json")));
            f.open(QIODevice::WriteOnly | QIODevice::Truncate);
            QByteArray d = JSON::stringify(jsonData).toUtf8();
            if (f.write(d) != d.size()) {
                qWarning() << "Database: Failed to write json completly";
            }
            f.close();
        }
    }
}

void Database::importFromJSON(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Database: failed to change to " << dir.absolutePath();
        return;
    }

    qDebug() << "Database: Import JSON Documents from" << path;

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
        if (error) {
            qWarning() << "\tNot a json file although json file extension!";
            continue;
        }

        // Prepare document
        if (!jsonData.contains(QLatin1String("plugin_"))) {
            jsonData.insert(QLatin1String("plugin_"), QLatin1String("server"));
        }
        if (!jsonData.contains(QLatin1String("instanceid_")) && jsonData.value(QLatin1String("type_")) != QLatin1String("collection")) {
            jsonData.insert(QLatin1String("instanceid_"), QLatin1String("0"));
        }
        jsonData.remove(QLatin1String("_rev"));

        // check for neccessary values before inserting into database
        if (!jsonData.contains(QLatin1String("plugin_")) ||
                jsonData[QLatin1String("plugin_")].toByteArray() == "AUTO") {
            qWarning() << "\tNo entry for plugin or plugin=AUTO. JSON Document not valid!";
            continue;
        }

        changeDocument(jsonData, false);
    }

    // recursivly go into all subdirectories
    const QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.size(); ++i) {
        dir.cd(dirs[i]);
        importFromJSON(dir.absolutePath());
        dir.cdUp();
    }
}

