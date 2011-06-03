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

#pragma once

#include <QtNetwork/QTcpServer>
#include <QVariantMap>
#include <QTimer>
#include <QSslSocket>

class HttpRequest;
class WebSocket;
class SessionExtension;
/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class HttpServer: public QTcpServer {
    Q_OBJECT
public:
    HttpServer();
    virtual ~HttpServer();
    bool start();
	static void writeDefaultHeaders(QSslSocket* socket);

private:
    virtual void incomingConnection(int socketDescriptor);
    QMap<QString, SessionExtension*> m_session_cache;
	QSet<WebSocket*> m_sessionLess_websockets;

private Q_SLOTS:
	// websocket
	void clearWebSocket(WebSocket* websocket);
	
	// http request
	void removeConnection(HttpRequest*);
	void headerParsed(HttpRequest*);
	
	// session extension
	void removeSession(SessionExtension*);
public Q_SLOTS:
    // service controller signals
    void dataSync(const QVariantMap& data, const QString& sessionid);
    // session controller signals
    SessionExtension* sessionBegin(QString sessionid);
    void sessionFinished(QString sessionid, bool timeout);
    SessionExtension* getSession(const QString& sessionid);
Q_SIGNALS:
    void dataReceived(QVariantMap,QString);
};
