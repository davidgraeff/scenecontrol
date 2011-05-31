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
        SessionExtension *c = sessionBegin(sessionid);
        if (c) c->dataChanged(cmdbytes);
    } else {
        foreach ( SessionExtension* c, m_session_cache ) {
            c->dataChanged ( cmdbytes );
        }
    }

}

SessionExtension* HttpServer::sessionBegin(QString sessionid) {
    SessionExtension* se = m_session_cache.value(sessionid);
    if (se) return se;

    Session* s = SessionController::instance()->getSession(sessionid);
    if (!s) return 0;
    se = new SessionExtension(this);
    m_session_cache.insert(sessionid,se);

    // session less websockets
    QMutableSetIterator<WebSocket*> i(m_sessionLess_websockets);
    while (i.hasNext()) {
        i.next();
        if (i.value()->getSessionID() == sessionid) {
            WebSocket* w = i.value();
            i.remove();
            disconnect(w, SIGNAL(removeWebSocket(WebSocket*)), this, SLOT(clearWebSocket(WebSocket*)));
            se->setWebsocket(w);
        }
    }
    return se;
}

void HttpServer::sessionFinished(QString sessionid, bool timeout) {
    Q_UNUSED(timeout);
    delete m_session_cache.take(sessionid);
}

void HttpServer::clearWebSocket(WebSocket* websocket) {
    websocket->deleteLater();
}

void HttpServer::removeConnection(HttpRequest* request) {
    request->deleteLater();
}

void HttpServer::headerParsed(HttpRequest* request) {
    if (request->httprequestType == HttpRequest::RequestTypeFile) {
        HttpResponseFile(request, this);

    } else if (request->httprequestType == HttpRequest::RequestTypeWebsocket) {

        WebSocket* w = WebSocket::makeWebsocket(request, this);
        request->deleteLater();
        if (!w) return;
        connect(w, SIGNAL(dataReceived(QVariantMap,QString)), SIGNAL(dataReceived(QVariantMap,QString)));
        connect(w, SIGNAL(removeWebSocket(WebSocket*)), SLOT(clearWebSocket(WebSocket*)));
        m_sessionLess_websockets.insert(w);

    } else if (request->httprequestType == HttpRequest::RequestTypePollJSon) {

        QByteArray data;
        SessionExtension* s = m_session_cache.value(request->m_sessionid);
        if (s) {
            QList< QByteArray > list = s->getDataCache();
            while (list.size()) {
                data.append(list.takeFirst()+ "\r\n");
            }
        }

        request->m_socket->write("HTTP/1.1 200 OK\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("Content-Length: "+QByteArray::number(data.size())+"\r\n");
		request->m_socket->write("Content-Type: application/json\r\n");
		request->m_socket->write("Cache-Control: max-age=0\r\n");
        request->m_socket->write("\r\n");
        request->m_socket->write(data);
        request->m_socket->flush();

    } else if (request->httprequestType == HttpRequest::RequestTypeSendJSon) {
        if (request->m_header.value("Content-Type") != "application/json")  {
            qWarning() << "Http Ajax Send: Only accept application/json";
            request->m_socket->write("HTTP/1.1 400 Bad request\r\n");
            request->m_socket->write("Content-Length: 0\r\n");
            HttpServer::writeDefaultHeaders(request->m_socket);
            request->m_socket->write("\r\n");
            request->m_socket->flush();
            return;
        }

        QByteArray data;
        const QByteArray line = request->m_socket->readAll().trimmed();

        // Ignore all but the first line
        if (line.size()) {
            bool ok = true;
            const QVariantMap json = QJson::Parser().parse (line, &ok).toMap();

            SessionController::SessionState c = SessionController::instance()->tryLoginAndResetSessionTimer(json, request->m_sessionid);
			
            if (c == SessionController::SessionValid)
                emit dataReceived(json, request->m_sessionid);
            else if (c == SessionController::SessionNewSessionDenied) {
                // login not possible. Auth thread does not accept new validation requests
                ServiceCreation sc = ServiceCreation::createNotification("sessioncontroller", "authentification.serverfull");
                data.append(QJson::Serializer().serialize(sc.getData()));
            } else if (c == SessionController::SessionInValid) {
                // There is no valid session for the given sessionid. Remember the client of this fact.
                ServiceCreation sc = ServiceCreation::createNotification("sessioncontroller", "authentification.failed");
                data.append(QJson::Serializer().serialize(sc.getData()));
				qDebug() << "compare session" <<  request->m_sessionid << request->m_header << line;
            } else if (c == SessionController::SessionRequestValidation) {
                ServiceCreation sc = ServiceCreation::createNotification("sessioncontroller", "authentification.temporary_sessionid");
				sc.setData("sessionid", request->m_sessionid);
                data.append(QJson::Serializer().serialize(sc.getData()));
			}
        }

        request->m_socket->write("HTTP/1.1 200 OK\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("Content-Length: "+QByteArray::number(data.size())+"\r\n");
		request->m_socket->write("Content-Type: application/json\r\n");
		request->m_socket->write("Cache-Control: max-age=0\r\n");
        request->m_socket->write("\r\n");
        request->m_socket->write(data);
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
