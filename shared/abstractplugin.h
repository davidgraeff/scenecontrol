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
#include <QLocalSocket>
#include <QLocalServer>
#include <QSet>
#include <QVariant>

/// Always define a PLUGIN_ID for this compilation unit
#ifndef PLUGIN_ID
#error Define PLUGIN_ID before including this header!
#endif

/// The name of the server communication socket
#define COMSERVERSTRING "server"

#include "shared/pluginservicehelper.h"

/**
 * Use this as your plugin base class. You have to define PLUGIN_ID before
 * including the header of this class.
 *
 * Communication:
 * You may use sendCmdToPlugin and sendDataToPlugin to communicate with other
 * plugins or with the server process (use plugin_id="server") and you have
 * to implement dataFromPlugin for receiving data from other processes.
 *
 * Reimplementable methods:
 * You may reimplement initialize, clear or configChanged to react on server state
 * changes. configChanged in only called by the server process for configuration
 * (key, value)-pairs that you saved by the method changeConfig before.
 *
 * Events:
 * If your plugin provides events (like current time == alarm time etc) you may
 * implement register_event and unregister_event and call eventTriggered if your
 * event is triggered.
 *
 * Conditions, Actions:
 * Just implement normal qt slots with up to 8 parameters
 * like "void dim_light(int id, int value)" or "bool isOn(int id)"
 */
class AbstractPlugin: public QLocalServer {
    Q_OBJECT
public:
    /**
     * Create local socket for incoming connections and connects to the server socket.
     * Return false if connection to server failed otherwise true.
     *
     * Use this in your plugin main method.
     */
    bool createCommunicationSockets();
    AbstractPlugin();
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) = 0;
    bool sendDataToPlugin(const QByteArray& plugin_id, const QVariantMap& data);
    void changeConfig(const QByteArray& key, const QVariantMap& data);
    void changeProperty(const QVariantMap& data, int sessionid = -1);
    void eventTriggered(const QString& eventid, const QString& dest_collectionId);
private Q_SLOTS:
    void readyReadCommunication();
    void newConnectionCommunication ();
    // If disconnected from server, quit plugin process
    void disconnectedFromServer();
protected:
    int m_lastsessionid;
private:
    QByteArray m_chunk;
    QMap<QByteArray, QLocalSocket*> m_connectionsByID;
    QMap<QLocalSocket*, QByteArray> m_connectionsBySocket;
    QSet<QLocalSocket*> m_pendingConnections;
    QLocalSocket* getClientConnection(const QByteArray& plugin_id);
    void writeToSocket(QLocalSocket* socket, const QVariantMap& data);
    int invokeHelperGetMethodId(const QByteArray& methodName);
    int invokeHelperMakeArgumentList(int methodID, const QVariantMap& inputData, QVector< QVariant >& output);
    QVariant invokeSlot(const QByteArray& methodname, int numParams, const char* returntype, QVariant p0, QVariant p1, QVariant p2, QVariant p3, QVariant p4, QVariant p5, QVariant p6, QVariant p7, QVariant p8);
public Q_SLOTS:
    /**
     * Return pluginid
     */
    QString pluginid() {
        return QLatin1String(PLUGIN_ID);
    }

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

    /**
    * All events of a collection are registered by their specific plugin.
    * A method of your plugin is called if an event
    * of the collection referenced by "collectionuid" should be registered
    * within your plugin.
    * If internally the event is triggered, you should call eventTriggered to nofiy
    * the server and execute the collection eventually.

    * When collections get removed they unregister their events with this method. If collections are changed
    * they will call unregister_event and register_event in sequence.
    */
    virtual void unregister_event ( const QString& eventid ) {
        Q_UNUSED(eventid);
    }
};
