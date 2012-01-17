#include "plugin_interconnect.h"
#include <QDebug>

PluginInterconnect::PluginInterconnect()
{
    connect(this, SIGNAL(newConnection()), SLOT(newConnection()));
    const QString name = QLatin1String("roomcontrol") + QLatin1String(PLUGIN_ID);
    removeServer(name);
    if (!listen(name)) {
        qWarning() << "Plugin interconnect server for" << PLUGIN_ID << "failed";
        return;
    }
}

void PluginInterconnect::readyRead()
{
    QLocalSocket* socket = (QLocalSocket*)sender();

    if (m_pendingConnections.contains(socket)) {
        m_pendingConnections.remove(socket);
        QByteArray l = socket->readLine();
        QList<QByteArray> ll = l.split('\t');
        if (ll.size() < 2 || ll[0] != "PLUGINID" || ll[1].trimmed().isEmpty()) {
            socket->deleteLater();
            return;
        }
        const QByteArray& plugin_id = ll[1].trimmed();
        m_connectionsByID[plugin_id] = socket;
        m_connectionsBySocket[socket] = plugin_id;
    }

    const QByteArray& plugin_id = m_connectionsBySocket.value(socket);
    if (plugin_id.isEmpty()) {
        qWarning() << "Plugin interconnect receiving failed" << PLUGIN_ID;;
        socket->deleteLater();
        return;
    }

    while (socket->canReadLine()) {
        QByteArray l = socket->readLine();
        dataFromPlugin(plugin_id, l);
    }
}

void PluginInterconnect::newConnection()
{
    while (hasPendingConnections()) {
        QLocalSocket * c = nextPendingConnection ();
        m_pendingConnections.insert(c);
        connect(c, SIGNAL(readyRead()), SLOT(readyRead()));
    }
}

bool PluginInterconnect::sendCmdToPlugin(const QByteArray& plugin_id, const QByteArray& data)
{
    QLocalSocket* socket = getClientConnection(plugin_id);
    if (!socket)
      return false;
    QDataStream stream(socket);
    // send payload to other plugin
    stream << data << '\n';
    return true;
}

bool PluginInterconnect::sendDataToPlugin(const QByteArray& plugin_id, const QVariant& data)
{
    QLocalSocket* socket = getClientConnection(plugin_id);
    if (!socket)
      return false;
    QDataStream stream(socket);
    // send payload to other plugin
    stream << data << '\n';
    return true;
}

QLocalSocket* PluginInterconnect::getClientConnection(const QByteArray& plugin_id) {
    // If this connection is known we get a valid socket out of the map (id->socket)
    QLocalSocket* socket = m_connectionsByID.value(plugin_id);
    // Try to connect to the target plugin if no connection is made so far
    if (!socket) {
        socket = new QLocalSocket();
        socket->connectToServer(QLatin1String("roomcontrol") + plugin_id);
        connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
	// wait for at least 30 seconds for a connection
        if (!socket->waitForConnected()) {
            delete socket;
	    return 0;
        }
        // connection established: add to map, send welcome string with current plugin id
        m_connectionsByID[plugin_id] = socket;
        m_connectionsBySocket[socket] = plugin_id;
	socket->write(QLatin1String("PLUGINID\t") + QLatin1String(PLUGINID) + QLatin1String("\n"));
    }
    return socket;
}

