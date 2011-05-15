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
#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QDir>
#include <QtXml/QDomDocument>
#include <QFileSystemWatcher>
#undef PLUGIN_ID
#define PLUGIN_ID "servicecontroller"
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>
#include <shared/abstractserver.h>

class CollectionInstance;
class Collections;
class PluginController;

struct ServiceStruct {
	QSet<CollectionInstance*> inCollections;
	QVariantMap data;
	AbstractPlugin_services* plugin;
};

class ServiceController: public QObject, public AbstractServer, public AbstractPlugin, public AbstractPlugin_services {
    Q_OBJECT
    PLUGIN_MACRO
public:
    ServiceController ();
    virtual ~ServiceController();
    void setPluginController ( PluginController* pc );

    /**
     * Remove services from m_valid_services that are using the plugin referenced by pluginid.
     */
    void removeServicesUsingPlugin ( const QString& pluginid );
	/**
	 * Check for services not referenced in collections
	 */
    void removeUnusedServices(bool warning);

    /**
     * Return service with uid
     */
    ServiceStruct* service ( const QString& uid );
	CollectionInstance* getCollection ( const QString& uid );
    PluginController* getPluginController();

    const QMap<QString, ServiceStruct*> &valid_services() const;

    void load ( bool service_dir_watcher );
	QVariantMap cloneService(ServiceStruct* service);
private:
    PluginController* m_plugincontroller;

	// loading
    QFileSystemWatcher m_dirwatcher;
	QList<QVariantMap> m_CollectionloadCache;
	
    // services
    QMap<QString, ServiceStruct*> m_valid_services; // uid -> data+plugin
    QMap< QString, CollectionInstance* > m_collections;
    bool removeMissingServicesFromCollection(QVariantMap& data, bool withWarning);
	void syncCollection(const QVariantMap& data, bool saveToDisk);

    // services
    QString serviceFilename ( const QString& id, const QString& uid );
    void saveToDisk ( const QVariantMap& data );
    /**
     * Only validated services are propagated to the respective plugin
     * for execution. In m_valid_services are only validated services.
     */
    bool validateService ( const QVariantMap& data );
    QString generateUniqueID();

    // routing
    QMap<QString, QSet<QString> > m_propertyid_to_plugins;

    /////////////// server interface ///////////////
    virtual void event_triggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid = "" );
    virtual void execute_action ( const QVariantMap& data, const char* pluginid = "" );
    virtual void property_changed ( const QVariantMap& data, const QString& sessionid = QString(), const char* pluginid = "" );
    virtual void register_listener ( const QString& unqiue_property_id, const char* pluginid = "" );
    virtual void unregister_all_listeners ( const char* pluginid = "" );
    virtual void unregister_listener ( const QString& unqiue_property_id, const char* pluginid = "" );

    /////////////// AbstractPlugin, AbstractPlugin_services ///////////////
    virtual void clear();
    virtual void initialize();
    virtual bool condition ( const QVariantMap& data, const QString& sessionid );
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid );
    virtual void unregister_event ( const QVariantMap& data, const QString& collectionuid );
    virtual void execute ( const QVariantMap& data, const QString& sessionid );
    virtual QList<QVariantMap> properties ( const QString& sessionid );
    EventMap<int> m_state_events; //state->set of uids
public Q_SLOTS:
    /**
     * Validates data to plugin description xml.
     * If data is valid and unique service id is set ("uid"=...) then add it to m_valid_services and save it to disk.
     * If the execution flag is set ("__execute"=true) and data describes an action ("__type"=action)
     * then do not add data to m_valid_services but call \link executeService.
     * \param service data
     * \param sessionid sessionid that caused this change or empty if not triggered by external sources like network
     */
    bool changeService ( const QVariantMap& unvalidatedData, const QString& sessionid, bool loading = false );

    /**
     * Remove service from m_valid_services and from disk and propagate that through the dataSync signal
     * \param uid unique service id
     */
    void removeService ( const QString& uid, bool removeFileOnly = false );

    /**
     * Execute action described by data (delegate to plugin).
     * Precondition: Data is checked
     */
    void executeAction ( const QVariantMap& data, const QString& sessionid );
    /**
     * Execute action in m_services with given uid immediately.
     */
    void executeActionByUID ( const QString& uid, const QString& sessionid );
    bool directoryChanged ( QString file, bool loading = false );

    /**
     * Session manager: A valid session started. Send all plugin infos via dataSync.
     */
    void sessionBegin ( const QString& sessionid );
    void sessionFinished ( QString sessionid, bool timeout );

Q_SIGNALS:
    /**
     * Emitted after a service has changed
     */
    void dataSync ( const QVariantMap& data, const QString& sessiondid = QString() );
    /**
     * Emitted after all services have been loaded from disk.
     */
    void dataReady();
    /**
     * Event triggered
     */
    void eventTriggered ( const QString& event_id, const QString& destination_collectionuid );
};
