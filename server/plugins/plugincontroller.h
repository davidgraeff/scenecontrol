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
#include <QStringList>
#include <QLocalSocket>
#include <QLocalServer>
#include "shared/jsondocuments/scenedocument.h"
#include <libdatastorage/datastorage.h>

class PluginController;
class StorageNotifierConfiguration: public AbstractStorageNotifier {
public:
	StorageNotifierConfiguration(PluginController* pluginController);
private:
	PluginController* mPluginController;
	// Called by the DataStorage
	virtual void documentChanged(const QString& filename, SceneDocument* oldDoc, SceneDocument* newDoc);
	// Called by the DataStorage
	virtual void documentRemoved(const QString& filename, SceneDocument* document);
};


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
    friend class StorageNotifierConfiguration;
	friend class iterator;
	friend class PluginProcess;
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
	 * To iterate over all plugins use this Iterator and PluginController::begin().
	 * The iterator offers the function "eof" to determine if the end of the iterator is reached.
	 */
	class iterator : public std::iterator<std::forward_iterator_tag, int>
	{
		PluginController* p;
		QMap<QString, QMap<QString,PluginProcess*> >::const_iterator i;
		QMap<QString,PluginProcess*>::const_iterator j;
	public:
		iterator(PluginController* x) :p(x) {i = p->m_plugins.constBegin();j=i.value().begin();}
		iterator(const iterator& mit) : p(mit.p),i(mit.i),j(mit.j) {}
		iterator& operator++() {
			if (i==p->m_plugins.end())
				return *this;
			++j;
			if (j==i.value().end()) {
				++i;
				j=i.value().begin();
			}
			return *this;
		}
		iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
		bool operator==(const iterator& rhs) {return p==rhs.p && i==rhs.i && j==rhs.j;}
		bool operator!=(const iterator& rhs) {return p!=rhs.p || i!=rhs.i || j!=rhs.j;}
		PluginProcess* operator*() {return j.value();}
		bool eof() {return i==p->m_plugins.end();}
	};
	iterator begin() {return iterator(this);}

	/**
	 * Request all properties from all known plugin processes.
	 */
    void requestAllProperties(int sessionid = -1);
	/**
	 * Request properties from a specified plugin.
	 */
	void requestProperty ( const SceneDocument& property, int sessionid = -1 );
	
	/**
	 * Get a list of all plugin ids. Example item: [plugin123, plugin234]
	 */
	QStringList pluginids() const;
	
	/**
	 * Execute a scene document if the component id + instanceID matches plugins.
	 * Return how many plugins matched.
	 * If you expect a response from a plugin, you may set the responseCallbackObject argument and provide a responeID.
	 * Your object have to implement a slot with the signature:
	 * 		const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid.
	 * The response is asynchronous. This method is thread safe.
	 */
	int execute(const SceneDocument& data, const QByteArray responseID = QByteArray(), QObject* responseCallbackObject = 0);
	int execute(const SceneDocument& data, int sessionid);
private:
    PluginController ();
    void startPluginProcessByConfiguration ( const SceneDocument* configuration );
	/**
	 * Return the plugin communication channel given by the plugin id and instance id
	 */
	PluginProcess* getPlugin(const QString& pluginID, const QString& instanceID);
	QList<PluginProcess*> getPlugins(const QString& pluginID, const QString instanceID = QString());
	/**
	 * A plugin process finished
	 */
	void processFinished(PluginProcess* process);
	/**
	 * Wait for all plugin processes to get finished or killed and exit the server
	 */
	void waitForPluginsAndExit();
	/**
	 * Finish a plugin process
	 */
	void removePluginInstance(const QString& pluginID, const QString instanceID);
	
	StorageNotifierConfiguration* mStorageNotifierConfiguration;
	QMap<QString, QMap<QString,PluginProcess*> > m_plugins;
    QSet<PluginProcess*> m_pluginprocesses;
	/// For the plugin iterator methods
    int m_index;
	/// Local server socket for plugins
    QLocalServer m_comserver;
	QMutex mExecuteMutex;
	bool m_exitIfNoPluginProcess;
	QStringList m_pluginlist; // list of all installed plugin processes including the not running plugins
private Q_SLOTS:
    void newConnection();
Q_SIGNALS:
	void pluginInstanceLoaded(const QString& componentid, const QString& instanceid);
};
