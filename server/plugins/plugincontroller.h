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
	 * Return false if the QLocalServer is not in listen state
	 */
	bool valid();
	/**
	 * Scan plugin application directory and request configuration for all available
	 * plugins.
	 */
	void scanPlugins();
	/**
	 * Wait for all plugin processes to get finished or killed and exit the server
	 */
	void waitForPluginsAndExit();
	/**
	 * A plugin process finished
	 */
    void processFinished(PluginProcess* process);

	/**
	 * Return the plugin communication channel given by the plugin id
	 */
    PluginProcess* getPlugin(const QString& pluginid, const QString& instanceid);

	/**
	 * To iterate over all plugins use getPluginIterator and nextPlugin
	 */
    QMap<QString,PluginProcess*>::iterator getPluginIterator();
    PluginProcess* nextPlugin(QMap<QString,PluginProcess*>::iterator& index);

	/**
	 * Request all properties from all known plugin processes
	 * and write them out to the server socket.
	 */
    void requestAllProperties(int sessionid = -1);
private:
    PluginController ();
    QMap<QString,PluginProcess*> m_plugins;
    QSet<PluginProcess*> m_pluginprocesses;
    QMap<QString, PluginProcess*> m_registeredevents;
	/// For the plugin iterator methods
    int m_index;
	/// Local server socket for plugins
    QLocalServer m_comserver;
	bool m_exitIfNoPluginProcess;
private Q_SLOTS:
    void newConnection();
public Q_SLOTS:
	void databaseStateChanged();
    void Event_add(const QString& id, const QVariantMap& event_data);
    void Event_remove(const QString& id);
	/**
	 * Start a plugin instance. The @configuration map has to contain at least the field: instanceid_
	 */
    bool startPluginInstance(const QString& pluginid, const QVariantMap& configuration);

};