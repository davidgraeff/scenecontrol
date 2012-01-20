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

	Purpose: Load plugins, load description xmls, route services and properties
*/

#pragma once
#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QSet>
#include <QSocketNotifier>
#include <QSslSocket>
#include <QTcpServer>

class libwebsocket;
class libwebsocket_context;
class ServiceController;

class WebSocket: public QTcpServer {
    Q_OBJECT
public:
    static WebSocket* instance();
    virtual ~WebSocket();
    void addWebsocketFD(int fd, short int direction);
    void removeWebsocketFD(int fd);
    /**
     * Received text (rawdata) from websocket (wsi)
     */
    void websocketReceive(const QByteArray& rawdata, libwebsocket* wsi);

    void sendToAllClients(const QByteArray& rawdata);
    void sendToClient(const QByteArray& rawdata, );
private:
    WebSocket ();
    ServiceController* m_servicecontroller;
    struct libwebsocket_context* m_websocket_context;
    QMap<int, QSocketNotifier*> m_websocket_fds;
    QMap<int, QSslSocket*> m_sockets;
    virtual void	incomingConnection ( int socketDescriptor );
private Q_SLOTS:
    void readyRead();
    void socketDisconnected();
    void sslErrors ( const QList<QSslError> & errors );
public slots:
    void websocketactivity(int);
Q_SIGNALS:
    void requestExecution ( const QVariantMap& data, );
};
