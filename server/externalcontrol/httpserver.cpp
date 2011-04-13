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
#include <QtNetwork/QSslSocket>
#include <QStringList>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QByteArray>
// json
#include <serializer.h>
#include <parser.h>

#include "paths.h"
#include "servicecontroller.h"
#include "config.h"
#include "clientconnection.h"

HttpServer::HttpServer() {}

HttpServer::~HttpServer()
{
    qDeleteAll(m_connections);
    m_connections.clear();
    qDeleteAll(m_temp_connections);
	m_temp_connections.clear();
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
            socket->deleteLater();
        } else {
            socket->setLocalCertificate(certificateFile(QLatin1String("server.crt")));
            if (!QDir().exists(certificateFile(QLatin1String("server.key")))) {
                qWarning()<<"Couldn't load private key"<<certificateFile(QLatin1String("server.key"));
                socket->deleteLater();
            } else {
                socket->setPrivateKey(certificateFile(QLatin1String("server.key")));
                socket->setPeerVerifyMode(QSslSocket::VerifyNone);
                socket->startServerEncryption();
                ClientConnection* c = new ClientConnection(socket);
                connect(c,SIGNAL(dataReceived(QVariantMap,QString)),SIGNAL(dataReceived(QVariantMap,QString)));
                connect(c,SIGNAL(removeConnection(ClientConnection*)),SLOT(removeConnection(ClientConnection*)));
                m_temp_connections.append ( c );
                //qDebug() << "connection"<<m_connections.size();
            }
        }
    }
    else
    {
        qWarning() << "Failed to bind incoming connection " << ROOM_LISTENPORT << "!";
        delete socket;
    }
}


void HttpServer::dataSync(const QVariantMap& data, const QString& sessionid) {
    if (m_connections.isEmpty()) return;
    const QByteArray cmdbytes = QJson::Serializer().serialize(data);
    if (sessionid.size()) {
        ClientConnection *c = m_connections.value(sessionid);
        if (c) c->writeJSON(cmdbytes);
    } else {
        foreach ( ClientConnection* c, m_connections ) {
            c->writeJSON ( cmdbytes );
        }
    }

}

void HttpServer::sessionBegin(QString sessionid) {
    ClientConnection* c = 0;
    for (int index=0;index<m_temp_connections.size();++index) {
        if (m_temp_connections[index]->sessionid == sessionid) {
            c = m_temp_connections.takeAt(index);
            break;
        }
    }
    if (!c) return;
    c->sessionEstablished();
    m_connections.insert(c->sessionid,c);
}

void HttpServer::sessionFinished(QString sessionid, bool timeout) {
    Q_UNUSED(timeout);
    foreach ( ClientConnection* c, m_connections ) {
        if (c->sessionid == sessionid) {
            removeConnection(c);
        }
    }
}

void HttpServer::removeConnection(ClientConnection* c) {
    //qDebug() << "disconnected"<<m_connections.size();
    m_temp_connections.removeAll(c);
    m_connections.remove(c->sessionid);
    c->deleteLater();
}
