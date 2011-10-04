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
#include <QSet>

#define PLUGIN_MACRO \
protected: \
	virtual QString pluginid() { return QLatin1String(PLUGIN_ID); } \
	AbstractServer* m_server; \
public: \
	virtual void connectToServer(AbstractServer* server) {m_server=server; } \

class ServiceID {
public:
    static QString id(const QVariantMap& data) {
        return data[QLatin1String("_id")].toString();
    }
    static QString pluginid(const QVariantMap& data) {
        return data[QLatin1String("plugin_")].toString();
    }
//     static void setId(QVariantMap& data, const QString& id) {
//         data[QLatin1String("_id")] = id;
//     }

    static QString collectionid(const QVariantMap& data) {
        return data.value(QLatin1String("collection_")).toString();
    }
    
    static bool isAction(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("action");
    }
    static bool isCondition(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("condition");
    }
    static bool isEvent(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("event");
    }
    static bool isCollection(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("collection");
    }
    static bool isExecutable(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("execute");
    }
    static bool isRemoveCmd(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("remove");
    }
    static bool isNotification(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("notification");
    }
    static bool isModelItem(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString() == QLatin1String("model");
    }
    static bool isNegatedCondition(const QVariantMap& data){
		return data.value(QLatin1String("conditionnegate_")).toBool();
	}
    static QString conditionGroup(const QVariantMap& data){
		QString cg = data.value(QLatin1String("conditiongroup_")).toString();
		if (cg.isEmpty()) cg = QLatin1String("all");
		return cg;
	}
	
	static QString type(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString();
    }

    static bool isMethod(const QVariantMap& data, const char* id) {
		return data[QLatin1String("member_")].toString() == QLatin1String(id);
	}
    static QString pluginmember(const QVariantMap& data) {
        return data[QLatin1String("member_")].toString();
    }

	static void setPluginmember(QVariantMap& data, const QString& uid) {
        data[QLatin1String("member_")] = uid;
    }
    
    /**
	 * Write toCollection property
	 */
//     static void setToCollection(QVariantMap& data, const QString& collectionuid) {
//         data[QLatin1String("__toCollection")] = collectionuid;
//     }
//     
    /**
	 * For eventMap
	 */
    static QVariantMap newDataWithCollectionUid(const QVariantMap& data, const QString& collectionuid) {
		QVariantMap modifieddata = data;
		modifieddata[QLatin1String("collection_")] = collectionuid;
        return modifieddata;
    }
};

class AbstractServer;
class AbstractPlugin
{
public:
    /**
     * Hint: Do not reimplement this method (It is implemented within the PLUGIN_MACRO macro).
     */
	virtual QString pluginid() = 0;
	
    /**
	 * Hint: Do not reimplement this method (It is implemented within the PLUGIN_MACRO macro).
     */
    virtual void connectToServer(AbstractServer* server) = 0;

    /**
     * (Re)Initialize the plugin. Called after all plugins are loaded but before the
     * network is initiated or by request from a client with sufficient access rights.
     */
	virtual void initialize() = 0;
	
    /**
     * Called by server process before it releases all ressources and finish.
     * Tidy up here.
     */
    virtual void clear() = 0;
	
};
Q_DECLARE_INTERFACE(AbstractPlugin, "com.roomcontrol.Plugin/2.0")
