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
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>
#include <shared/abstractserver.h>

class PluginController;
class ServiceController: public QObject, public AbstractServer, public AbstractPlugin, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
public:
    struct ServiceStruct {
        QVariantMap data;
        AbstractPlugin_services* plugin;
    };

    ServiceController ();
    virtual ~ServiceController();
    void setPluginController(PluginController* pc);

    /**
     * Remove services from m_valid_services that are using the plugin referenced by pluginid.
     */
    void removeServicesUsingPlugin(const QString& pluginid);

    /**
     * Return service with uid
     */
    ServiceStruct* service(const QString& uid);

    const QMap<QString, ServiceStruct*> &valid_services() const;

    void load(bool service_dir_watcher);
private:
    PluginController* m_plugincontroller;
    QFileSystemWatcher m_dirwatcher;
    // services
    QMap<QString, ServiceStruct*> m_valid_services; // uid -> data+plugin

    // services
    QString serviceFilename(const QString& id, const QString& uid);
    void saveToDisk(const QVariantMap& data );
    /**
     * Only validated services are propagated to the respective plugin
     * for execution. In m_valid_services are only validated services.
     */
    bool validateService(const QVariantMap& data );
    void setUniqueID(QVariantMap& data);

    // routing
    QMap<QString, QSet<QString> > m_propertyid_to_plugins;

    /////////////// server interface ///////////////
    virtual void event_triggered(const QString& event_id, const char* pluginid = "");
    virtual void execute_action(const QVariantMap& data, const char* pluginid = "");
    virtual void property_changed(const QVariantMap& data, const QString& sessionid = QString(), const char* pluginid = "");
    virtual void register_listener(const QString& unqiue_property_id, const char* pluginid = "");
    virtual void unregister_all_listeners(const char* pluginid = "");
    virtual void unregister_listener(const QString& unqiue_property_id, const char* pluginid = "");

	/////////////// AbstractPlugin, AbstractPlugin_services ///////////////
    virtual void clear();
    virtual void initialize();
    virtual bool condition(const QVariantMap& data);
    virtual void event_changed(const QVariantMap& data);
    virtual void execute(const QVariantMap& data);
    virtual QList<QVariantMap> properties(const QString& sessionid);
	QMap<int, QSet<QString> > m_state_events; //state->set of uids
public Q_SLOTS:
    /**
     * Validates data to plugin description xml.
     * If data is valid and unique service id is set ("uid"=...) then add it to m_valid_services and save it to disk.
     * If the execution flag is set ("__execute"=true) and data describes an action ("__type"=action)
     * then do not add data to m_valid_services but call \link executeService.
     * \param service data
     * \param sessionid sessionid that caused this change or empty if not triggered by external sources like network
     */
    void changeService(const QVariantMap& data, const QString& sessionid);

    /**
     * Remove service from m_valid_services and from disk and propagate that through the dataSync signal
     * \param uid unique service id
     */
    void removeService(const QString& uid);

    /**
     * Execute action described by data (delegate to plugin).
     * Precondition: Data is checked
     */
    void executeAction(const QVariantMap& data);
    /**
     * Execute action in m_services with given uid immediately.
     */
    void executeActionByUID(const QString& uid);
    void directoryChanged(QString file, bool loading = false);

    /**
     * Session manager: A valid session started. Send all plugin infos via dataSync.
     */
    void sessionBegin(QString sessionid);
Q_SIGNALS:
    /**
     * Emitted after a service has changed
     */
    void dataSync(const QVariantMap& data, const QString& sessiondid = QString());
    /**
     * Emitted after all services have been loaded from disk.
     */
    void dataReady();
    /**
     * Event triggered
     */
    void eventTriggered(const QString& event_id);
};
