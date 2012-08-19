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
#include <QByteArray>
#include <QTextStream>

/**
 * A scene document is the in-memory copy of a json document, technically implemented
 * with a QVariantMap. This class provides a lot of convenience functions to access typical
 * scene document properties like "id_", "type_" or "pluginid_".
 */
class SceneDocument {
private:
    QVariantMap m_map;
public:
    /** 
     * Constructor: Construct by an existing QVariantMap
     */
    SceneDocument(const QVariantMap& map);
    /** 
     * Constructor: Construct by a json document
     */
    SceneDocument(const QByteArray& jsondata);
    /** 
     * Constructor: Construct by a stream containing json
     */
    SceneDocument(const QTextStream& jsonstream);
    
    /**
     * Creates a model item remove notification.
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     */
    static SceneDocument createModelRemoveItem(const char* id) ;

    /**
     * Creates a model item change notification.
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     */
    static SceneDocument createModelChangeItem(const char* id) ;

    /**
     * Creates a model reset notification.
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     * \param key Model key/index item name
     */
    static SceneDocument createModelReset(const char* id, const char* key) ;

    /**
     * Creates a notification
     * \param id Notification id. Must be the same as documented in the plugin xml file.
     */
    static SceneDocument createNotification(const char* id) ;

    static SceneDocument createRemoveByUidCmd(const QString& uid, const QString& type) ;

    static SceneDocument createExecuteByUidCmd(const QString& uid) ;

    /**
     * Creates an execute cmd. Will only be propagated to the destination plugin and executed if all necessary data is set.
     * \param plugin_id The destination plugin that implements the wanted functionality.
     * \param id The id for this action within the destination plugin
     */
    static SceneDocument createExecuteByDataCmd(const char* plugin_id, const char* id) ;
    
    /***************** Is valid ******************/
    bool isValid();
    /***************** Export to json ******************/
    QByteArray getjson();
    
    /***************** Getter ******************/
    QString toString(const char* key);
    int toInt(const char* key) ;
    bool toBool(const char* key) ;
    double toDouble(const char* key) ;
    QVariantList toList(const char* key) ;
    QVariantMap toMap(const char* key) ;

    QVariantMap& getData() ;

    /***************** Setter ******************/
    void setData(const char* index, const QVariant& data) ;
    /**
      * Correct QVariant types if neccessary because javascript/qt-qml don't have fixed types e.g. int:0 == double:0
      * Return false if not all values can be converted. Return true and change the internal data if successful.
      * @param types Example: ("some_key":"int" , "some_other_key": "string")
      */
    bool correctTypes(const QVariantMap& types);
	
    /***************** Convinience Getter/Setter ******************/
    QString id() {
        return m_map[QLatin1String("id_")].toString();
    }
    void setid(const QString& id) {
      m_map[QLatin1String("id_")] = id;
    }

    QString pluginid() {
        return m_map[QLatin1String("pluginid_")].toString();
    }
    void setPluginid(const QByteArray& pluginid) {
        m_map[QLatin1String("pluginid_")] = pluginid;
    }
    bool hasPluginid() {
        return m_map.contains(QLatin1String("pluginid_"));
    }
    
    QString plugininstance() {
        return m_map[QLatin1String("plugininstance_")].toString();
    }
    void setPlugininstance(const QString& instanceid) {
        m_map[QLatin1String("plugininstance_")] = instanceid;
    }
    
    QString type() {
        return m_map[QLatin1String("type_")].toString();
    }
    bool hasType() {
        return m_map.contains(QLatin1String("type_"));
    }
    
    QString sceneid() {
        return m_map.value(QLatin1String("collection_")).toString();
    }
    void setSceneid(const QString& collectionid) {
        m_map[QLatin1String("collection_")] = collectionid;
    }

    static QString configurationkey() {
        return data.value(QLatin1String("configkey_")).toString();
    }
    static void setConfigurationkey(const QByteArray& configurationkey) {
        m_map[QLatin1String("configkey_")] = configurationkey;
    }

    enum TypeEnum {
        TypeUnknown,
	// Documents that are stored to disk and need to have an ID
        TypeEvent,
        TypeCondition,
        TypeAction,
        TypeCollection,
        TypeConfiguration,
        TypeSchema,
	
	// Special types
        TypeExecution, 	// Same as Action but is executed immediately and not stored
        TypeRemove,	// Remove stored document
        TypeNotification,// A notification
        TypeModelItem	// An item of a model
    };

    static bool checkType(const checkTypeEnum t) {
        const QByteArray type = m_map.value(QLatin1String("type_")).toByteArray();
        return (
			(type == "action" && t==TypeAction) ||
			(type == "condition" && t==TypeCondition) ||
			(type == "event" && t==TypeEvent) ||
			(type == "collection" && t==TypeCollection) ||
			(type == "execute" && t==TypeExecution) ||
			(type == "remove" && t==TypeRemove) ||
			(type == "notification" && t==TypeNotification) ||
			(type == "model" && t==TypeModelItem) ||
			(type == "configuration" && t==TypeConfiguration)
		);
    }

    bool isNegatedCondition() {
        return m_map.value(QLatin1String("conditionnegate_")).toBool();
    }
    QString conditionGroup() {
        QString cg = m_map.value(QLatin1String("conditiongroup_")).toString();
        if (cg.isEmpty()) cg = QLatin1String("all");
        return cg;
    }


    bool isMethod(const const char* id) {
        return m_map[QLatin1String("method_")].toByteArray() == QByteArray(id);
    }
    QByteArray method() {
        return m_map[QLatin1String("method_")].toByteArray();
    }
    void setMethod(const QByteArray& methodname) {
        m_map[QLatin1String("method_")] = methodname;
    }
    bool hasMethod() {
        return m_map.contains(QLatin1String("method_"));
    }

    int sessionid() {
        return m_map.value(QLatin1String("sessionid_"),-1).toInt();
    }

    void removeSessionID() {
        m_map.remove(QLatin1String("sessionid_"));
    }

    void setSessionID(const int sessionid) {
        m_map[QLatin1String("sessionid_")] = sessionid;
    }

    bool isQtSlotRespons() {
        return m_map.contains(QLatin1String("isrespons_"));
    }

    void setQtSlotRespons() {
        m_map[QLatin1String("isrespons_")] = true;
    }
};