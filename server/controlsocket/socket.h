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
#include "shared/jsondocuments/scenedocument.h"
#include <libdatastorage/datastorage.h>

class ServiceController;


class StorageNotifierSocket: public AbstractStorageNotifier {
public:
	StorageNotifierSocket(int sessionid);
private:
	int m_sessionid;
	// Called by the DataStorage
	virtual void documentChanged(const QString& filename, SceneDocument* oldDoc, SceneDocument* newDoc);
	// Called by the DataStorage
	virtual void documentRemoved(const QString& filename, SceneDocument* document);
};

class ControlServerSocket: public QTcpServer {
    Q_OBJECT
public:
    static ControlServerSocket* instance();
    virtual ~ControlServerSocket();

	/**
	 * Send a document as json formated string to all connected clients (sessionid==-1)
	 * or to a specific client.
	 */
    void sendToClients (const QByteArray& rawdata, int sessionid = -1);
	/**
	 * Incoming connections are pure tcp and not ssl encrypted if you call this method.
	 * For debug purposes only! SSL Clients are not able to connect after activing this.
	 */
    void disableSecureConnections();
private:
    ControlServerSocket ();
    ServiceController* m_servicecontroller;
    QMap<int, QSslSocket*> m_sockets;
	QMap<int, StorageNotifierSocket*> m_notifiers;
    bool m_disabledSecureConnections;
    virtual void	incomingConnection ( int socketDescriptor );
	StorageNotifierSocket* notifier(int sessionid);
private Q_SLOTS:
    void readyRead();
    void socketDisconnected();
    void sslErrors ( const QList<QSslError> & errors );
};
