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

*/

#pragma once
#include <QVariantMap>
#include <QString>

// getter
#define MAP(ITEMID) data[QLatin1String(ITEMID)].toMap()
#define LIST(ITEMID) data[QLatin1String(ITEMID)].toList()
#define DATA(ITEMID) data[QLatin1String(ITEMID)].toString()
#define INTDATA(ITEMID) data[QLatin1String(ITEMID)].toInt()
#define BOOLDATA(ITEMID) data[QLatin1String(ITEMID)].toBool()
#define DOUBLEDATA(ITEMID) data[QLatin1String(ITEMID)].toDouble()

class ServiceType {
public:
    static QString uniqueID(const QVariantMap& data) {
        return data[QLatin1String("__uid")].toString();
    }
    static void setUniqueID(QVariantMap& data, const QString& uid) {
        data[QLatin1String("__uid")] = uid;
    }

    static bool isAction(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("action");
    }
    static bool isCondition(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("condition");
    }
    static bool isEvent(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("event");
    }
    static bool isCollection(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("collection");
    }
    static bool isExecutable(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("execute");
    }
    static bool isRemoveCmd(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("remove");
    }
    static bool isNotification(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("notification");
    }
    static bool isModelItem(const QVariantMap& data) {
        return data[QLatin1String("__type")].toString() == QLatin1String("model");
    }
};

class ServiceCreation {
private:
    QVariantMap m_map;
    ServiceCreation() {}
public:
	/**
	 * Creates a model item remove notification.
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createModelRemoveItem(const char* plugin_id, const char* id) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("id")] =  QLatin1String(id);
		sc.m_map[QLatin1String("__plugin")] = QLatin1String(plugin_id);
        sc.m_map[QLatin1String("__type")] = QLatin1String("model");
		sc.m_map[QLatin1String("__event")] = QLatin1String("remove");
        return sc;
    }
    
	/**
	 * Creates a model item change notification.
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createModelChangeItem(const char* plugin_id, const char* id) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("id")] =  QLatin1String(id);
		sc.m_map[QLatin1String("__plugin")] = QLatin1String(plugin_id);
        sc.m_map[QLatin1String("__type")] = QLatin1String("model");
		sc.m_map[QLatin1String("__event")] = QLatin1String("change");
        return sc;
    }
    
	/**
	 * Creates a model reset notification.
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createModelReset(const char* plugin_id, const char* id) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("id")] =  QLatin1String(id);
		sc.m_map[QLatin1String("__plugin")] = QLatin1String(plugin_id);
        sc.m_map[QLatin1String("__type")] = QLatin1String("model");
		sc.m_map[QLatin1String("__event")] = QLatin1String("reset");
        return sc;
    }
    
	/**
	 * Creates a notification
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createNotification(const char* plugin_id, const char* id) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("id")] =  QLatin1String(id);
		sc.m_map[QLatin1String("__plugin")] = QLatin1String(plugin_id);
        sc.m_map[QLatin1String("__type")] = QLatin1String("notification");
        return sc;
    }

    static ServiceCreation createRemoveByUidCmd(const QString& uid) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("__uid")] = uid;
        sc.m_map[QLatin1String("__type")] = QLatin1String("remove");
        return sc;
    }

    static ServiceCreation createExecuteByUidCmd(const QString& uid) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("__uid")] = uid;
        sc.m_map[QLatin1String("__type")] = QLatin1String("execute");
        return sc;
    }
    
	/**
	 * Creates an execute cmd. Will only be propagated to the destination plugin and executed if all necessary data is set.
	 * \param plugin_id The destination plugin that implements the wanted functionality.
	 * \param id The id for this action within the destination plugin
	 */
    static ServiceCreation createExecuteByDataCmd(const char* plugin_id, const char* id) {
        ServiceCreation sc;
        sc.m_map[QLatin1String("id")] = QString(QLatin1String(plugin_id) + QLatin1String("_") + QLatin1String(id));
        sc.m_map[QLatin1String("__type")] = QLatin1String("execute");
        return sc;
    }
    
    void setData(const char* index, const QVariant& data) {
        m_map[QLatin1String(index)] = data;
    }

    QVariantMap getData() {
        return m_map;
    }

};

/**
 * Plugin interface for dealing with actions, conditions, events and properties
 */
class AbstractPlugin_services
{
public:
    /**
     * Return current state of all plugin properties. The server
     * reguests all properties from all plugins after a client has connected.
     * Example in the VariantMap: fancyplugin_ledIsOn = true
     * Note: Properties are temporary and are not saved by the server.
     * You should cache longer-to-generate properties. This call
     * should not block the server noticable!
     * \param sessionid id of the client session that requests properties of this plugin
     */
    virtual QList<QVariantMap> properties(const QString& sessionid) = 0;
    /**
     * Implement execution routines for all provided actions
     */
    virtual void execute(const QVariantMap& data) = 0;

    /**
    * Implement check routines for all provided conditions
    */
    virtual bool condition(const QVariantMap& data) = 0;

    /**
    * Event data loaded by the server or changed by a client. Implement routines to trigger the actual event based on these data
    * and remove the event with the event id (from data) that was established by previous data.
    */
    virtual void event_changed(const QVariantMap& data) = 0;
};
Q_DECLARE_INTERFACE(AbstractPlugin_services, "com.roomcontrol.PluginServices/2.0")
