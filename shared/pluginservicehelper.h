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


// getter
#define MAP(ITEMID) data[QLatin1String(ITEMID)].toMap()
#define LIST(ITEMID) data[QLatin1String(ITEMID)].toList()
#define DATA(ITEMID) data[QLatin1String(ITEMID)].toString()
#define INTDATA(ITEMID) data[QLatin1String(ITEMID)].toInt()
#define BOOLDATA(ITEMID) data[QLatin1String(ITEMID)].toBool()
#define DOUBLEDATA(ITEMID) data[QLatin1String(ITEMID)].toDouble()

class ServiceData {
private:
    QVariantMap m_map;
    ServiceData() {}
public:
    ServiceData(const QVariantMap& map) : m_map(map) {}
    /**
     * Creates a model item remove notification.
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     */
    static ServiceData createModelRemoveItem(const char* id) ;

    /**
     * Creates a model item change notification.
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     */
    static ServiceData createModelChangeItem(const char* id) ;

    /**
     * Creates a model reset notification.
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     * \param key Model key/index item name
     */
    static ServiceData createModelReset(const char* id, const char* key) ;

    /**
     * Creates a notification
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     */
    static ServiceData createNotification(const char* id) ;

    static ServiceData createRemoveByUidCmd(const QString& uid, const QString& type) ;

    static ServiceData createExecuteByUidCmd(const QString& uid) ;

    /**
     * Creates an execute cmd. Will only be propagated to the destination plugin and executed if all necessary data is set.
     * \param plugin_id The destination plugin that implements the wanted functionality.
     * \param id The id for this action within the destination plugin
     */
    static ServiceData createExecuteByDataCmd(const char* plugin_id, const char* id) ;

    void setData(const char* index, const QVariant& data) ;

    QVariantMap& getData() ;

    static QString id(const QVariantMap& data) {
        return data[QLatin1String("_id")].toString();
    }
    static QString string(const QVariantMap& data, const char* key) {
        return data[QString::fromAscii(key)].toString();
    }
    static QString idChangeSeq(const QVariantMap& data) {
        return data[QLatin1String("id")].toString();
    }

    static QString pluginid(const QVariantMap& data) {
        return data[QLatin1String("plugin_")].toString();
    }
    static void setPluginid(QVariantMap& data, const QByteArray& pluginid) {
        data[QLatin1String("plugin_")] = pluginid;
    }
    void setPluginid(const QByteArray& pluginid) {
        m_map[QLatin1String("plugin_")] = pluginid;
    }

    static QString collectionid(const QVariantMap& data) {
        return data.value(QLatin1String("collection_")).toString();
    }
    static void setCollectionid(QVariantMap& data, const QByteArray& collectionid) {
        data[QLatin1String("collection_")] = collectionid;
    }

    static QString value(const QVariantMap& data) {
        return data.value(QLatin1String("value_")).toString();
    }
    static void setValue(QVariantMap& data, const QVariant& value) {
        data[QLatin1String("value_")] = value;
    }

    static QString configurationkey(const QVariantMap& data) {
        return data.value(QLatin1String("configkey_")).toString();
    }
    static void setConfigurationkey(QVariantMap& data, const QByteArray& configurationkey) {
        data[QLatin1String("configkey_")] = configurationkey;
    }

    enum checkTypeEnum {
        TypeUnknown,
        TypeEvent,
        TypeCondition,
        TypeAction,
        TypeCollection,
        TypeExecution,
        TypeRemove,
        TypeNotification,
        TypeModelItem,
        TypeConfiguration
    };
    Q_DECLARE_FLAGS(checkTypeEnums, checkTypeEnum)

    static bool checkType(const QVariantMap& data, checkTypeEnums t) {
        checkTypeEnum ct = TypeUnknown;
        const QByteArray type = data[QLatin1String("type_")].toByteArray();
        if (type == "action") ct = TypeAction;
        else if (type == "condition") ct = TypeCondition;
        else if (type == "event") ct = TypeEvent;
        else if (type == "collection") ct = TypeCollection;
        else if (type == "execute") ct = TypeExecution;
        else if (type == "remove") ct = TypeRemove;
        else if (type == "notification") ct = TypeNotification;
        else if (type == "model") ct = TypeModelItem;
        else if (type == "configuration") ct = TypeConfiguration;
        return t.testFlag(ct);
    }

    static bool isNegatedCondition(const QVariantMap& data) {
        return data.value(QLatin1String("conditionnegate_")).toBool();
    }
    static QString conditionGroup(const QVariantMap& data) {
        QString cg = data.value(QLatin1String("conditiongroup_")).toString();
        if (cg.isEmpty()) cg = QLatin1String("all");
        return cg;
    }

    static QString type(const QVariantMap& data) {
        return data[QLatin1String("type_")].toString();
    }

    static bool isMethod(const QVariantMap& data, const char* id) {
        return data[QLatin1String("member_")].toByteArray() == QByteArray(id);
    }

    static QByteArray method(const QVariantMap& data) {
        return data[QLatin1String("member_")].toByteArray();
    }

    static void setMethod(QVariantMap& data, const QByteArray& methodname) {
        data[QLatin1String("member_")] = methodname;
    }

    static bool hasMethod(const QVariantMap& data) {
        return data.contains(QLatin1String("member_"));
    }

    static int sessionid(const QVariantMap& data) {
        return data.value(QLatin1String("sessionid_"),-1).toInt();
    }

    static void removeSessionID(QVariantMap& data) {
        data.remove(QLatin1String("sessionid_"));
    }

    static void setSessionID(QVariantMap& data, const int sessionid) {
        data[QLatin1String("sessionid_")] = sessionid;
    }

    static bool isQtSlotRespons(const QVariantMap& data) {
        return data.contains(QLatin1String("isrespons_"));
    }

    static void setQtSlotRespons(QVariantMap& data) {
        data[QLatin1String("isrespons_")] = true;
    }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ServiceData::checkTypeEnums)
