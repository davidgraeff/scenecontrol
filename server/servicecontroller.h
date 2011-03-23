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
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>
#include <shared/abstractserver.h>

class ServiceController: public QObject, public AbstractServer
{
    Q_OBJECT
public:
    ServiceController ();
    virtual ~ServiceController();
    /**
     * For server objects to register their interface. E.g. the backup object to
     * execute backup actions and send changed properties to \link ServiceController.
     * For validation and service description the server.xml file is used.
     */
    void useServerObject(AbstractPlugin*);
private:
    // services
    QMap<QString, QVariantMap> m_valid_services;

    // services
    QString serviceFilename(const QString& id, const QString& uid);
    void saveToDisk(const QVariantMap& data );
    void removeFromDisk( const QVariantMap& data );
    /**
     * Only validated services are propagated to the respective plugin
     * for execution. In m_valid_services are only validated services.
     */
    bool validateService( const QVariantMap& data );

    // plugins
    QList<AbstractPlugin*> m_plugins;
    QMap<QString, AbstractPlugin_services*> m_plugin_services;
    QMap<QString, AbstractPlugin_otherproperties*> m_plugin_otherproperties;
    QMap<QString, AbstractPlugin_settings*> m_plugin_settings;
    QMap<QString, QDomDocument> m_pluginxml;
    QMap<QString, QString> m_idToPlugin;

    /**
     * Load plugins and their corresponding xml description file and
     * registers all provided properties and services
     */
    void loadPlugins();
    void loadServerXML();

    // routing
    QMap<QString, QSet<QString> > m_propertyid_to_plugins;

    // server interface
    virtual void event_triggered(const QString& event_id, const char* pluginid = "") {
        emit eventTriggered(event_id);
    }
    virtual void execute_action(const QVariantMap& data, const char* pluginid = "") {
        executeService(data);
    }
    virtual void property_changed(const QVariantMap& data, const QString& sessionid = QString(), const char* pluginid = "") {
        emit dataSync(data, false, sessionid);
        QSet<QString> plugins = m_propertyid_to_plugins.value(DATA("id"));
        foreach(QString pluginid, plugins) {
            AbstractPlugin_otherproperties* plugin = m_plugin_otherproperties.value(pluginid);
            if (plugin) plugin->otherPropertyChanged(data, sessionid);
        }
    }
    virtual void register_listener(const QString& unqiue_property_id, const char* pluginid = "") {
        m_propertyid_to_plugins[unqiue_property_id].insert(QString::fromAscii(pluginid));
    }
    virtual void unregister_all_listeners(const char* pluginid = "") {
        const QString id = QString::fromAscii(pluginid);
        QMutableMapIterator<QString, QSet<QString> > it(m_propertyid_to_plugins);
        while (it.hasNext()) {
            it.value().remove(id);
            if (it.value().isEmpty())
                it.remove();
        }
    }
    virtual void unregister_listener(const QString& unqiue_property_id, const char* pluginid = "") {
        m_propertyid_to_plugins[unqiue_property_id].remove(QString::fromAscii(pluginid));
        if (m_propertyid_to_plugins[unqiue_property_id].isEmpty())
            m_propertyid_to_plugins.remove(unqiue_property_id);
    }
public Q_SLOTS:
    /**
     * Validates data to plugin description xml.
     * If data is valid and unique service id is set ("uid"=...) then add it to m_valid_services and save it to disk.
     * If the execution flag is set ("__execute"=true) and data describes an action ("__type"=action)
     * then do not add data to m_valid_services but call \link executeService.
     * \param service data
     */
    void changeService(const QVariantMap& data);

    /**
     * Remove service from m_valid_services and from disk and propagate that through the dataSync signal
     * \param uid unique service id
     */
    void removeService(const QString& uid);

    /**
     * Execute service described by data immediately.
     */
    void executeService(const QVariantMap& data);
    /**
     * Execute service in m_services with given uid immediately.
     */
    void executeService(const QString& uid);
Q_SIGNALS:
    /**
     * Emitted after a service has changed
     */
    void dataSync(const QVariantMap& data, bool removed = false, const QString& sessiondid = QString());
    /**
     * Emitted after all services have been loaded from disk.
     */
    void dataReady(const QMap<QString, QVariantMap>& data_map);
    /**
     * Event triggered
     */
    void eventTriggered(const QString& event_id);
};
