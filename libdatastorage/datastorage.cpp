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
    unload();
}

void Database::unload()
{

}

Database *Database::instance()
{
    if (databaseInstance == 0)
        databaseInstance = new Database();

    return databaseInstance;
}


void Database::load()
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
        std::unique_ptr<mongo::DBClientCursor> cursor =
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
        std::unique_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query("roomcontrol.event", BSON("collection_" << collecion_id.toStdString()));
        while ( cursor->more() ) {
            servicelist.append(BJSON::fromBson(cursor->next()));
        }
    }
    {
        std::unique_ptr<mongo::DBClientCursor> cursor =
            m_mongodb.query("roomcontrol.condition", BSON("collection_" << collecion_id.toStdString()));
        while ( cursor->more() ) {
            servicelist.append(BJSON::fromBson(cursor->next()));
        }
    }
    {
        std::unique_ptr<mongo::DBClientCursor> cursor =
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
        std::unique_ptr<mongo::DBClientCursor> cursor =
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
    std::unique_ptr<mongo::DBClientCursor> cursor =
        m_mongodb.query("roomcontrol.schema", mongo::BSONObj());
    while ( cursor->more() ) {
        QVariantMap jsonData = BJSON::fromBson(cursor->next());
        emit doc_changed(ServiceData::id(jsonData), jsonData);
    }
}

void Database::requestCollections()
{
    std::unique_ptr<mongo::DBClientCursor> cursor =
        m_mongodb.query("roomcontrol.collection", mongo::BSONObj());
    while ( cursor->more() ) {
        QVariantMap jsonData = BJSON::fromBson(cursor->next());
        emit doc_changed(ServiceData::id(jsonData), jsonData);
    }
}

void Database::removeDocument(const QString &type, const QString &id)
{
    if (type.isEmpty()||id.isEmpty())
        return;

    m_mongodb.remove("roomcontrol."+type.toStdString(), BSON("_id" << id.toStdString()));
    if (type == QLatin1String("collection")) {
        // if it is a collection, remove all actions, conditions, events belonging to it
        // Do it in a per-element way to update the listen collection after every operation
        std::unique_ptr<mongo::DBClientCursor> cursor;
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
    std::unique_ptr<mongo::DBClientCursor> cursor =
        m_mongodb.query(dbid, query, 1);
    if (cursor.get()==0) {
        qWarning()<<"Database: Query failed!" << type << id;
        return false;
    }
    return cursor->itcount();
}

