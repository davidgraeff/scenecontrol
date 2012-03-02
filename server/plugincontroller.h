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

	Purpose: Start, stop plugin processes. Provide the server communication
	socket for plugins to connect to. Hold references to all
	plugin communication channels. Route added, changed, removed event documents
	and configurations to their respective plugin.
*/

#pragma once
#include <QtCore/QObject>
#include <QVariantMap>
#include <QLocalSocket>
#include <QLocalServer>

class PluginProcess;
class PluginCommunication;

/**
 * Purpose: Start, stop plugin processes. Provide the server communication
 * socket for plugins to connect to. Hold references to all
 * plugin communication channels. Route added, changed, removed event documents
 * and configurations to their respective plugin.
 */
class PluginController: public QObject
{
    Q_OBJECT
public:
	/// Singleton object
    static PluginController* instance();
    virtual ~PluginController();
	/**
	 * Start all applications in a well-known path. If those processes do not
	 * establish a communication channel to the server within a given time bound
	 * kill those processes again.
	 * Return false if the server socket can not be created (e.g. exist already)
	 */
    bool startplugins();
	/**
	 * Kill plugin process and unregister all events associated with this plugin
	 */
    void unloadPlugin(const QString& id);
	/**
	 * Plugin failed to establish a valid communication channel and will be killed.
	 * Used to be called by the PluginCommunication helper object.
	 */
    void removePluginFromPending(PluginCommunication* pluginprocess);
	/**
	 * A plugin successfully established a valid communication channel
	 */
    void addPlugin(const QString& id, PluginCommunication* pluginprocess);
    void removeProcess(PluginProcess* process);

	/**
	 * Return the plugin communication channel given by the plugin id
	 */
    PluginCommunication* getPlugin(const QString& pluginid);

	/**
	 * To iterate over all plugins use getPluginIterator and nextPlugin
	 */
    QMap<QString,PluginCommunication*>::iterator getPluginIterator();
    PluginCommunication* nextPlugin(QMap<QString,PluginCommunication*>::iterator& index);

	/**
	 * Request all properties from all known plugin processes
	 * and write them out to the server socket.
	 */
    void requestAllProperties(int sessionid = -1);
private:
    PluginController ();
    QMap<QString,PluginCommunication*> m_plugins;
    QMap<QLocalSocket*,PluginCommunication*> m_pendingplugins;
    QSet<PluginProcess*> m_pluginprocesses;
    QMap<QString, PluginCommunication*> m_registeredevents;
	/// For the plugin iterator methods
    int m_index;
	/// Local server socket for plugins
    QLocalServer m_comserver;
private Q_SLOTS:
    void newConnection();
public Q_SLOTS:
    void Event_add(const QString& id, const QVariantMap& event_data);
    void Event_remove(const QString& id);
    void settings(const QString& pluginid, const QString& key, const QVariantMap& data);
};
