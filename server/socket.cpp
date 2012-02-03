#include "socket.h"
#include "paths.h"
#include "config.h"
#include <QDebug>
#include <QSslKey>
#include <shared/json.h>
#include "plugincontroller.h"
#include "pluginprocess.h"
//openssl req -x509 -new -out server.crt -keyout server.key -days 365

#define __FUNCTION__ __FUNCTION__

Socket::~Socket()
{
}

Socket::Socket() {
    if (listen(QHostAddress::Any, ROOM_LISTENPORT)) {
        qDebug() << "SSL TCPSocket Server ready on port" << ROOM_LISTENPORT;
    }
}

static Socket* websocket_instance = 0;
Socket* Socket::instance()
{
    if (!websocket_instance)
        websocket_instance = new Socket();
    return websocket_instance;
}

void Socket::incomingConnection(int socketDescriptor)
{
    QSslSocket *socket = new QSslSocket;
    if (socket->setSocketDescriptor(socketDescriptor)) {
        m_sockets.insert(socketDescriptor, socket);
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(socket, SIGNAL(sslErrors (QList<QSslError>)), this, SLOT(sslErrors (QList<QSslError>)));
        socket->ignoreSslErrors();
        socket->setProtocol(QSsl::SslV3);

        QByteArray key;
        QByteArray cert;

        QFile fileKey(setup::certificateFile("server.key"));
        if (fileKey.open(QIODevice::ReadOnly))
        {
            key = fileKey.readAll();
            fileKey.close();
        }
        else
        {
            qWarning() << fileKey.errorString();
        }

        QFile fileCert(setup::certificateFile("server.crt"));
        if (fileCert.open(QIODevice::ReadOnly))
        {
            cert = fileCert.readAll();
            fileCert.close();
        }
        else
        {
            qWarning() << fileCert.errorString();
        }

        QSslKey sslKey(key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "1234");
        if (key.isNull()) {
            qWarning() << "key invalid";
        }

        QSslCertificate sslCert(cert);
        if (sslCert.isNull()) {
            qWarning() << "sslCert invalid";
        }
        socket->setPrivateKey(sslKey);
        socket->setLocalCertificate(sslCert);
        socket->startServerEncryption();
        qDebug() << "new socket" << socketDescriptor << socket->state() << socket->peerAddress() << socket->sslErrors();

        // Notify plugins of new session
        PluginController* pc = PluginController::instance();
        QMap<QString,PluginCommunication*>::iterator i = pc->getPluginIterator();
        while (PluginCommunication* plugin = pc->nextPlugin(i)) {
            plugin->session_change(socketDescriptor, true);
        }
    } else {
        delete socket;
    }
}

void Socket::sslErrors ( const QList<QSslError> & errors ) {
    qDebug() << errors;
}

void Socket::readyRead() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
    while (serverSocket->canReadLine()) {
        const QByteArray rawdata = serverSocket->readLine();
        if (!rawdata.length())
            continue;
        QVariant v =JSON::parse(rawdata);
        if (!v.isNull()) {
            emit requestExecution(v.toMap(), serverSocket->socketDescriptor());
            serverSocket->write("{\"response\":0, \"msg\":\"OK\"}\n");
        } else {
            serverSocket->write("{\"response\":1, \"msg\":\"Failed to parse json\"}\n");
        }
    }
}

void Socket::socketDisconnected() {
    QSslSocket *socket = (QSslSocket *)sender();
    const int socketDescriptor = socket->socketDescriptor();
    m_sockets.remove(socketDescriptor);
    socket->deleteLater();

    // Notify plugins of finished session
    PluginController* pc = PluginController::instance();
    QMap<QString,PluginCommunication*>::iterator i = pc->getPluginIterator();
    while (PluginCommunication* plugin = pc->nextPlugin(i)) {
        plugin->session_change(socketDescriptor, false);
    }

    qDebug() << "socket closed" << socketDescriptor << socket->errorString() << socket->error();
}

void Socket::sendToAllClients(const QByteArray& rawdata) {
    // send data over sockets
    QMap<int, QSslSocket*>::const_iterator it = m_sockets.constBegin();
    for (;it != m_sockets.constEnd();++it) {
        it.value()->write(rawdata);
    }
}

void Socket::sendToClient(const QByteArray& rawdata, int sessionid) {
    // try to find a socket connection with corresponding sessionid
    QSslSocket *socket = m_sockets.value(sessionid);
    if (socket) {
        socket->write(rawdata);
        return;
    } else {
     qWarning() << "SessionID misused! No socket found" << sessionid; 
    }
}

void Socket::propagateProperty(const QVariantMap& data, int sessionid) {
    QByteArray jsondata = JSON::stringify(data);
    qDebug()<<__FUNCTION__<<jsondata<<sessionid;
    if (!jsondata.isEmpty()) {
        if (sessionid==-1)
            sendToAllClients(jsondata + "\n");
        else
            sendToClient(jsondata + "\n", sessionid);
    } else if (data.size()) {
        qWarning() << "Json Serializer failed at:" << data;
    }
}
