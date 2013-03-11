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

    Purpose: AbstractPlugin serves as the base class for all plugins.
    See documentation/plugin.* source code for an example usage.
*/

#pragma once
#include <QMap>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslKey>
#include <QSet>
#include <QVariant>

#include "../jsondocuments/scenedocument.h"

/**
 * Use this as your plugin base class. You have to define PLUGIN_ID before
 * including the header of this class.
 *
 * Communication:
 * You may use sendDataToComponent to communicate with other
 * plugins or with the server process (use componentid := COMSERVERSTRING). See
 * "Conditions, Actions" below for receiving.
 *
 * Reimplementable methods:
 * You may reimplement initialize, clear or configChanged to react on server state
 * changes. configChanged in only called by the server process for configuration
 * (key, value)-pairs that you saved by the method changeConfig before.
 *
 * Events:
 * An event can occur anytime and will trigger a scene execution.
 * If your plugin provides events (like current time == alarm time etc) you may
 * implement methods (QT slots) that expect the scene id to be executed. Call eventTriggered if your
 * event is triggered. The server may unregister events (if the destination scene gets removed etc)
 * by calling the exact same method but with an empty scene id.
 *
 * Conditions, Actions:
 * Just implement methods (qt slots with up to 8 parameters)
 * like "void dim_light(int id, int value)" or "bool isOn(int id)". Conditions may only
 * return a boolean response value.
 */
class AbstractPlugin: public QSslSocket {
    Q_OBJECT
public:
    /**
     * Create local socket for incoming connections and connects to the server socket.
     * Return false if connection to server failed otherwise true.
     *
     * Use this in your plugin main method.
     */
	static AbstractPlugin* createInstance(const QByteArray& pluginid, const QByteArray& instanceid, const QByteArray& serverip, const QByteArray& port);
    virtual ~AbstractPlugin();
	/**
	 * If data from the server has been received you can get the clients id (session id)
	 * that initiated the data request. This method returns -1 if no client, but the server
	 * send data.
	 */
	int getLastSessionID();
	bool callRemoteComponent( const QVariantMap& dataout );
    void changeConfig(const QByteArray& category, const QVariantMap& data);
    void changeProperty(const QVariantMap& data, int sessionid = -1);
    void eventTriggered(const QString& eventid, const QString& dest_collectionId);
private Q_SLOTS:
    void readyReadCommunication();
    // If disconnected from server, quit plugin process
    void disconnectedFromServer();
	void sslErrors ( const QList<QSslError> & errors );
protected:
    int m_lastsessionid;
	QString m_pluginid;
	QString m_instanceid;
private:
	AbstractPlugin(const QString& pluginid, const QString& instanceid);
	QSslKey readKey(const QString& fileKeyString);
	QSslCertificate readCertificate(const QString& filename);
	
    int invokeHelperGetMethodId(const QByteArray& methodName);
	// Return -1 if parameters are not matching
    int invokeHelperMakeArgumentList(int methodID, const QVariantMap& inputData, QVector< QVariant >& output);
    QVariant invokeSlot(const QByteArray& methodname, int numParams, const char* returntype, QVariant p0, QVariant p1, QVariant p2, QVariant p3, QVariant p4, QVariant p5, QVariant p6, QVariant p7, QVariant p8);
	bool createCommunicationSockets(const QByteArray& serverip, int port);
public Q_SLOTS:
    /**
     * Return pluginid
     */
    QString pluginid();
    QString instanceid();
    
    /**
     * (Re)Initialize the plugin. Called after all plugins are loaded but before the
     * network is initiated.
     */
    virtual void initialize() {}

    /**
     * Called by server process before it releases all ressources and finish.
     * Tidy up here.
     */
    virtual void clear() {}

    /**
    * Called by server process if a new session with sessionid is starting (running=true)
    * or a session finished (running=false).
    */
    virtual void session_change ( int sessionid, bool running ) {
        Q_UNUSED(sessionid);
        Q_UNUSED(running);
    }
    /**
     * Settings have changed. This method is called at startup for initial settings, too.
     */
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data) {
        Q_UNUSED(configid);
        Q_UNUSED(data);
    };
    /**
     * Return current state of all plugin properties. The server
     * reguests all properties from all plugins after a client has connected.
     *
     * Example in the VariantMap: fancyplugin_ledIsOn = true
     *
     * Note: Properties are temporary and are not saved by the server.
     * You should cache longer-to-generate properties. This call
     * should not block the server noticable!
     * \param sessionid id of the client session that requests properties of this plugin
     */
    virtual void requestProperties(int sessionid) {
        Q_UNUSED(sessionid);
    };
};
