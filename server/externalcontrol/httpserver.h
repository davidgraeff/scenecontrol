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

class ClientConnection;
/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class HttpServer: public QTcpServer {
    Q_OBJECT
public:
    HttpServer();
    virtual ~HttpServer();
    bool start();

private:
    virtual void incomingConnection(int socketDescriptor);
	ClientConnection* findConnection(const QString& sessionid);
    QList<ClientConnection*> m_http_connections;
	QList<ClientConnection*> m_websocket_connections;
	QMap<QString, ClientConnection*> m_session_cache;
public Q_SLOTS:
    void removeConnection(ClientConnection* );
	void upgradedConnection();
    // service controller signals
    void dataSync(const QVariantMap& data, const QString& sessionid);
    // session controller signals
    void sessionBegin(QString sessionid);
    void sessionFinished(QString sessionid, bool timeout);
Q_SIGNALS:
    void dataReceived(QVariantMap,QString);
};
