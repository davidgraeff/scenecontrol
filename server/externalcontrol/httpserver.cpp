/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Purpose: https server
*/

#include "httpserver.h"
// QT
#include <QtNetwork/QHostAddress>
#include <QStringList>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QSettings>

#include "paths.h"
#include "servicecontroller.h"
#include "authThread.h"
#include "config.h"
#include <serializer.h>
#include <parser.h>


ClientConnection::ClientConnection(QSslSocket* s) {
    bufferpos=0;
    bufferBrakes=0;
    socket=s;
}
ClientConnection::~ClientConnection() {
    delete socket;
}

///////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer() : m_service(0) {}

HttpServer::~HttpServer()
{
    qDeleteAll(m_connections);
    m_connections.clear();
    close();
}

bool HttpServer::start()
{
    if ( !listen ( QHostAddress::Any, ROOM_LISTENPORT ) )
    {
        qCritical() << "TCP Server listen failed on port " << ROOM_LISTENPORT << "!";
        return false;
    } else
        qDebug() << "Listen on port" << ROOM_LISTENPORT;
    return true;
}

void HttpServer::sslErrors(QList< QSslError > errors) {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
    QString errorString;
    foreach(QSslError error, errors) {
        switch (error.error()) {
        default:
            errorString.append(error.errorString());
            errorString.append(QLatin1String(" "));
            break;
        case QSslError::SelfSignedCertificate:
            break;
        }
    }
    if (errorString.size())
        qWarning()<<__FUNCTION__ << socket->peerAddress()<< errorString;
}

void HttpServer::peerVerifyError(QSslError error) {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
    switch (error.error()) {
    default:
        qWarning()<<__FUNCTION__ << socket->peerAddress() << error.errorString();
        break;
    case QSslError::SelfSignedCertificate:
        break;
    }
}

/*
   * Generate a self signed root certificate
    openssl req -x509 -utf8 -newkey rsa:1024 -nodes -days 3650 -keyout server.key -out server.crt
    openssl -new -x509 -extensions v3_ca -days 3650 -keyout cakey.pem -out cacert.pem
    openssl x509 -noout -text -in thecert.pem
*/
void HttpServer::incomingConnection ( int socketDescriptor )
{
    QSslSocket *socket = new QSslSocket;
    if ( socket->setSocketDescriptor ( socketDescriptor ) == true )
    {
        if (!QDir().exists(certificateFile(QLatin1String("server.crt")))) {
            qWarning()<<"Couldn't load local certificate"<<certificateFile(QLatin1String("server.crt"));
        } else
            socket->setLocalCertificate(certificateFile(QLatin1String("server.crt")));
        if (!QDir().exists(certificateFile(QLatin1String("server.key")))) {
            qWarning()<<"Couldn't load private key"<<certificateFile(QLatin1String("server.key"));
        } else
            socket->setPrivateKey(certificateFile(QLatin1String("server.key")));

        socket->setPeerVerifyMode(QSslSocket::VerifyNone);
        socket->startServerEncryption();
        connect ( socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
        connect(socket, SIGNAL(peerVerifyError(const QSslError &)),
                this, SLOT(peerVerifyError (const QSslError &)));
        connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)),
                this, SLOT(sslErrors(const QList<QSslError> &)));
        connect(socket,SIGNAL(disconnected()),SLOT(disconnected()));
        m_connections.insert ( socket, new ClientConnection(socket) );
//         QByteArray data;
//         data.append("{\"type\" : \"serverinfo\", \"version\" : \""ROOM_NETWORK_APIVERSION"\", \"auth\" : \"required\", \"auth_timeout\" : \"");
//         data.append(QByteArray::number(ROOM_NETWORK_AUTHTIMEOUT));
//         data.append("\", \"plugins\" : \"");
//         data.append("\"}");
//         socket->write ( data );
        //qDebug() << "new connection, waiting for authentification";
    }
    else
    {
        qWarning() << "Failed to bind incoming connection " << ROOM_LISTENPORT << "!";
        delete socket;
    }
}

void HttpServer::syncClient(QSslSocket* socket)
{
    //sync all providers with the new client
    QList<QObject*> synclist;

//     const QList<ExecuteWithBase*> servicesList = m_service->m_servicesList;
//     foreach (ExecuteWithBase* p, servicesList)
//     {
//         synclist.append(p->base());
//     }
//     const QList<AbstractStateTracker*> stateTrackerlist = m_service->stateTracker();
//     foreach (AbstractStateTracker* p, stateTrackerlist)
//     {
//         synclist.append(p);
//     }
//
//     qDebug() << "New client from" << socket->peerAddress().toString().toUtf8().data() << "Sync objects:" << synclist.size();
//     foreach (QObject* p, synclist)
//     {
//         QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
//         QJson::Serializer serializer;
//         QByteArray cmdbytes = serializer.serialize(variant);
//         socket->write ( cmdbytes );
//     }
//     socket->write("{\"type\" : \"complete\"}");
//     socket->flush();
}

QByteArray HttpServer::getNextJson(ClientConnection* c)
{
    for (; c->bufferpos<c->buffer.size(); ++c->bufferpos)
    {
        if (c->buffer[c->bufferpos] == '{')
            ++c->bufferBrakes;
        else if (c->buffer[c->bufferpos] == '}')
            --c->bufferBrakes;

        if (c->bufferpos>1 && c->bufferBrakes == 0)
        {
            const QByteArray res = c->buffer.mid(0,c->bufferpos+1);
            c->buffer.remove(0,c->bufferpos+1);
            c->bufferpos = 0;
            return res;
        }
    }

    // no paragraph {..} found
    return QByteArray();
}

void HttpServer::readyRead()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    Q_ASSERT(socket);
    ClientConnection* c = m_connections.value(socket);
    if (!c) return;

    if ( socket->bytesAvailable() )
    {
        c->buffer += socket->readAll();
        // if buffer>2MByte discard
        if ( c->buffer.size() >1024*2000 )
        {
            c->buffer.clear();
            c->bufferBrakes = 0;
            c->bufferpos = 0;
            qWarning() << "receive incomplete" << c->buffer;
            return;
        }
    } else return;


    QByteArray cmdstring;
    while (1) {
        cmdstring = getNextJson(c);
        if (cmdstring.isEmpty())
        {
            return;
        }

        bool ok;
        QVariantMap result = QJson::Parser().parse (cmdstring, &ok).toMap();
        //TODO to service controller
    };
}

void HttpServer::disconnected()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    Q_ASSERT(socket);
    //qDebug() << "Client disconnected:" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor();
    m_connections.take ( socket )->deleteLater();
}

void HttpServer::setServiceController ( ServiceController* controller ) {
    m_service = controller;
}

void HttpServer::dataSync(const QVariantMap& data, bool removed, const QString& sessiondid) {
    if (m_connections.isEmpty()) return;
    const QByteArray cmdbytes = QJson::Serializer().serialize(data);
    foreach ( ClientConnection* c, m_connections ) {
        if (sessiondid.isEmpty() || sessiondid==c->sessionid) c->socket->write ( cmdbytes );
    }
}

void HttpServer::sessionAuthFailed(QString sessionid) {
//     if (ptr==0) { // Test
//         qDebug() << __FUNCTION__ << name;
//         return;
//     }
//     Q_UNUSED(name);
//     QSslSocket* socket = qobject_cast<QSslSocket*>(ptr);
//     Q_ASSERT(socket);
//     QByteArray data("{\"type\" : \"auth\", \"result\" : \"failed\"}");
//     socket->write ( data );
//     socket->flush();
}

void HttpServer::sessionBegin(QString sessionid) {
//     if (ptr==0) { // Test
//         qDebug() << __FUNCTION__ << name;
//         return;
//     }
//     QSslSocket* socket = qobject_cast<QSslSocket*>(ptr);
//     Q_ASSERT(socket);
//     ClientConnection* c = m_connections.value(socket);
//     Q_ASSERT(c);
//     c->setAuth(name);
//     QByteArray data("{\"type\" : \"auth\", \"result\" : \"ok\"}");
//     socket->write ( data );
//     syncClient(socket);
}

void HttpServer::sessionFinished(QString sessionid, bool timeout) {
//     QByteArray data("{\"type\" : \"auth\", \"result\" : \"timeout\"}");
//     socket->write ( data );
//     socket->flush();
//     qWarning() << "Client auth timeout:" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor();
//     m_connections.take ( socket )->deleteLater();
}

