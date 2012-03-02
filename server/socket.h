/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2012  David Gräff

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

	Purpose: Network ssl socket for
	(1) listening to plugin property changes
	(2) request	plugin action execution or collection execution
	(3) registering as remotesystem client (plugin: remotesystem)
*/

#pragma once
#include <QVariantMap>
#include <QSslSocket>
#include <QTcpServer>

class ServiceController;

class Socket: public QTcpServer {
    Q_OBJECT
public:
    static Socket* instance();
    virtual ~Socket();

    void sendToAllClients(const QByteArray& rawdata);
    void sendToClient(const QByteArray& rawdata, int sessionid );
    
    void propagateProperty (const QVariantMap& data, int sessionid = -1);
private:
    Socket ();
    ServiceController* m_servicecontroller;
    QMap<int, QSslSocket*> m_sockets;
    virtual void	incomingConnection ( int socketDescriptor );
private Q_SLOTS:
    void readyRead();
    void socketDisconnected();
    void sslErrors ( const QList<QSslError> & errors );
Q_SIGNALS:
    void requestExecution ( const QVariantMap& data, int session_id);
};
