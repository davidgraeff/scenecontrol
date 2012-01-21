#include "abstractplugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMetaMethod>

#define MAGICSTRING "roomcontrol_"

AbstractPlugin::AbstractPlugin()
{
    connect(this, SIGNAL(newConnection()), SLOT(newConnectionCommunication()));
}

bool AbstractPlugin::createCommunicationSockets()
{
    // create server socket for incoming connections
    const QString name = QLatin1String(MAGICSTRING) + QLatin1String(PLUGIN_ID);
    removeServer(name);
    if (!listen(name)) {
        qWarning() << "Plugin interconnect server for" << PLUGIN_ID << "failed";
        return false;
    }
    // create connection to server
    QLocalSocket* socketToServer = getClientConnection(COMSERVERSTRING);
    if (!socketToServer) {
        qWarning() << "Couldn't connect to server:" << PLUGIN_ID << "failed";
        return false;
    }
    connect(socketToServer, SIGNAL(disconnected()), SLOT(disconnectedFromServer()));
    return true;
}

void AbstractPlugin::readyReadCommunication()
{
    QLocalSocket* socket = (QLocalSocket*)sender();

    const QByteArray& plugin_id = m_connectionsBySocket.value(socket);
    if (plugin_id.isEmpty()) {
        qWarning() << PLUGIN_ID << "Receiving failed";
        socket->deleteLater();
        return;
    }
    while (socket->canReadLine()) {
        const QByteArray l = socket->readLine();
        QDataStream stream(l);
        QVariantMap variantdata;
        stream >> variantdata;
        const QByteArray method = ServiceData::method(variantdata);
        qDebug() << PLUGIN_ID << "Received from" << plugin_id << method << variantdata;

        if (method.size()) {
            // Predefined methods
            if (method == "pluginid") {
                QVariantMap dataout;
                ServiceData::setMethod(dataout, "pluginid");
                ServiceData::setPluginid(dataout, PLUGIN_ID);
                QDataStream streamout(socket);
                streamout << dataout << '\n';
            } else if (method == "changeConfig") {
                const QString key = ServiceData::configurationkey(variantdata);
                configChanged(key.toUtf8(), variantdata);
            } else if (method == "initialize") {
                initialize();
            } else if (method == "clear") {
                clear();
            } else if (method == "requestProperties") {
                const int sessionid = ServiceData::sessionid(variantdata);
                requestProperties(sessionid);
            } else if (method == "session_change") {
                const int sessionid = ServiceData::sessionid(variantdata);
                session_change(sessionid, variantdata.value(QLatin1String("running")).toBool());
            } else if (method == "unregister_event") {
                const QString eventid = variantdata.value(QLatin1String("eventid")).toString();
                unregister_event(eventid);
            } else {
                qDebug() << "call plugin slot" << PLUGIN_ID << method;
                // try to call the slot with method name
                const int c = metaObject()->methodCount();
                for (int i=0;i<c;++i) {
                    qDebug() << metaObject()->method(i).signature();
                }
                // If method not found call dataFromPlugin
                dataFromPlugin(plugin_id, variantdata);
            }
        } else
            dataFromPlugin(plugin_id, variantdata);
    }
}


void AbstractPlugin::newConnectionCommunication()
{
    while (hasPendingConnections()) {
        QLocalSocket * socket = nextPendingConnection ();
        // Accept only connections if their name start with roomcontrol_
        if (!socket->serverName().startsWith(QLatin1String(MAGICSTRING))) {
            socket->deleteLater();
            continue;
        }
        const QByteArray plugin_id = socket->serverName().mid(sizeof(MAGICSTRING)-1).toAscii();
        m_connectionsByID[plugin_id] = socket;
        m_connectionsBySocket[socket] = plugin_id;
    }
}

bool AbstractPlugin::sendCmdToPlugin(const QByteArray& plugin_id, const QByteArray& data)
{
    QLocalSocket* socket = getClientConnection(plugin_id);
    if (!socket)
        return false;
    QDataStream stream(socket);
    // send payload to other plugin
    stream << data << '\n';
    return true;
}

bool AbstractPlugin::sendDataToPlugin(const QByteArray& plugin_id, const QVariantMap& data)
{
    QLocalSocket* socket = getClientConnection(plugin_id);
    if (!socket) {
        qWarning() << PLUGIN_ID << "Failed to send data to" << plugin_id;
        return false;
    }
    QDataStream stream(socket);
    // send payload to other plugin
    stream << data << '\n';
    return true;
}

QLocalSocket* AbstractPlugin::getClientConnection(const QByteArray& plugin_id) {
    // If this connection is known we get a valid socket out of the map (id->socket)
    QLocalSocket* socket = m_connectionsByID.value(plugin_id);
    // Try to connect to the target plugin if no connection is made so far
    if (!socket) {
        socket = new QLocalSocket();
        socket->connectToServer(QLatin1String(MAGICSTRING) + plugin_id);
        connect(socket, SIGNAL(readyRead()), SLOT(readyReadCommunication()));
        // wait for at least 30 seconds for a connection
        if (!socket->waitForConnected()) {
            delete socket;
            return 0;
        }
        // connection established: add to map, send welcome string with current plugin id
        m_connectionsByID[plugin_id] = socket;
        m_connectionsBySocket[socket] = plugin_id;
    }
    return socket;
}

void AbstractPlugin::changeConfig(const QByteArray& key, const QVariantMap& data) {
    QVariantMap modifieddata;
    ServiceData::setMethod(modifieddata, "changeConfig");
    ServiceData::setPluginid(modifieddata, PLUGIN_ID);
    ServiceData::setConfigurationkey(modifieddata, key);
    ServiceData::setValue(modifieddata, data);
    sendDataToPlugin(COMSERVERSTRING, modifieddata);
}

void AbstractPlugin::changeProperty(const QVariantMap& data, int sessionid) {
    QVariantMap modifieddata;
    ServiceData::setMethod(modifieddata, "changeProperty");
    ServiceData::setPluginid(modifieddata, PLUGIN_ID);
    ServiceData::setValue(modifieddata, data);
    ServiceData::setSessionID(modifieddata, sessionid);
    sendDataToPlugin(COMSERVERSTRING, modifieddata);
}

void AbstractPlugin::eventTriggered(const QByteArray& eventid, const QByteArray& dest_collectionId) {
    QVariantMap modifieddata;
    ServiceData::setMethod(modifieddata, "eventTriggered");
    ServiceData::setPluginid(modifieddata, PLUGIN_ID);
    ServiceData::setCollectionid(modifieddata, dest_collectionId);
    modifieddata[QLatin1String("sourceevent")] = eventid;
    sendDataToPlugin(COMSERVERSTRING, modifieddata);
}

void AbstractPlugin::disconnectedFromServer() {
    QCoreApplication::exit(1);
}

