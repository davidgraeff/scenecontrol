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
#include "httprequest.h"
#include "websocket.h"
#include "session.h"
#include <sessioncontroller.h>
#include "httpresponsefile.h"

HttpServer::HttpServer() {

}

HttpServer::~HttpServer()
{
    qDeleteAll(m_session_cache);
    m_session_cache.clear();
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


void HttpServer::writeDefaultHeaders(QSslSocket* socket) {
    QByteArray date = QLocale(QLocale::English).toString(QDateTime::currentDateTime(), QLatin1String("ddd, d MMMM yyyy hh:mm:ss")).toAscii() + " GMT";
    socket->write("Server: roomcontrolserver\r\nDate: " + date + "\r\n" + "Content-Language: de\r\n");
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
                HttpRequest* r = new HttpRequest(socket, this);
                connect(r,SIGNAL(headerParsed(HttpRequest*)),SLOT(headerParsed(HttpRequest*)));
                connect(r,SIGNAL(removeConnection(HttpRequest*)),SLOT(removeConnection(HttpRequest*)));
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
    const QByteArray cmdbytes = QJson::Serializer().serialize(data);
    if (cmdbytes.isNull()) {
        qWarning()<<"JSON failed"<<data;
        return;
    }

    if (sessionid.size()) {
        SessionExtension *c = m_session_cache.value(sessionid);
        if (c) c->dataChanged(cmdbytes);
    } else {
        foreach ( SessionExtension* c, m_session_cache ) {
            c->dataChanged ( cmdbytes );
        }
    }

}

void HttpServer::sessionBegin(QString sessionid) {
    Session* s = SessionController::instance()->getSession(sessionid);
    if (!s) return;
    m_session_cache.insert(sessionid,new SessionExtension(this));
}

void HttpServer::sessionFinished(QString sessionid, bool timeout) {
    Q_UNUSED(timeout);
    delete m_session_cache.take(sessionid);
}

void HttpServer::clearWebSocket(WebSocket* websocket) {
    websocket->deleteLater();
}

void HttpServer::authentificatedWebSocket(WebSocket* websocket, const QString& sessionid) {
    SessionExtension* s = m_session_cache.value(sessionid);
    if (!s) return;
    disconnect(websocket, SIGNAL(removeWebSocket(WebSocket*)), this, SLOT(clearWebSocket(WebSocket*)));
    s->setWebsocket(websocket);
}

void HttpServer::removeConnection(HttpRequest* request) {
    request->deleteLater();
}

void HttpServer::headerParsed(HttpRequest* request) {
    if (request->httprequestType == HttpRequest::RequestTypeFile) {
        (new HttpResponseFile(request, this))->deleteLater();
    } else if (request->httprequestType == HttpRequest::RequestTypeWebsocket) {
        WebSocket* w = WebSocket::makeWebsocket(request, this);
        request->deleteLater();
        if (!w) return;
        connect(w, SIGNAL(authentificated(WebSocket*,QString)), SLOT(authentificatedWebSocket(WebSocket*,QString)));
        connect(w, SIGNAL(dataReceived(QVariantMap,QString)), SIGNAL(dataReceived(QVariantMap,QString)));
        connect(w, SIGNAL(removeWebSocket(WebSocket*)), SLOT(clearWebSocket(WebSocket*)));
    } else if (request->httprequestType == HttpRequest::RequestTypePollJSon) {
        request->m_socket->write("HTTP/1.1 200 OK\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("\r\n");

        SessionExtension* s = m_session_cache.value(request->m_sessionid);
        if (s) {
            QList< QByteArray > list = s->getDataCache();
            while (list.size()) {
                request->m_socket->write(list.takeFirst()+ "\r\n");
            }
        }
        request->m_socket->flush();
    } else if (request->httprequestType == HttpRequest::RequestTypeSendJSon) {
        while (request->m_socket->canReadLine()) {
            const QByteArray line = request->m_socket->readLine().trimmed();
            bool ok = true;
            const QVariantMap data = QJson::Parser().parse (line, &ok).toMap();
            if (!ok) {
                qWarning() << "client requested json per http and json parser failed" << line;
                continue;
            }
            if (request->m_authok) { // accept json commands only after successful login or for a login json command
                emit dataReceived(data, request->m_sessionid);
            } else if (!SessionController::instance()->tryLogin(data, request->m_sessionid)) {
                qWarning() << "client send json per http and login failed with json" << line;
            }
        }
        request->m_socket->write("HTTP/1.1 200 OK\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("\r\n");
        request->m_socket->flush();
    } else {
		qWarning()<<"Internal Server Error" << request->m_requestedfile;
        request->m_socket->write("HTTP/1.1 500 Internal Server Error\r\n");
        request->m_socket->write("Content-Length: 0\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("\r\n");
        request->m_socket->flush();
	}
}
