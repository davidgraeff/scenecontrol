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
    SceneDocument(const QVariantMap& map = QVariantMap());
    /** 
     * Constructor: Construct by a json document
     */
    SceneDocument(const QByteArray& jsondata);
    /** 
     * Constructor: Construct by a stream containing json
     */
    SceneDocument(QTextStream& jsonstream);
    
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
    bool isValid() const;
    /***************** Export to json ******************/
    QByteArray getjson() const;
    
    /***************** Getter ******************/
    QString toString(const char* key) const;
    int toInt(const char* key) const;
    bool toBool(const char* key) const;
    double toDouble(const char* key) const;
    QVariantList toList(const char* key) const;
    QVariantMap toMap(const char* key) const;

    QVariantMap& getData() ;
	const QVariantMap& getData() const;

    /***************** Setter ******************/
    void setData(const char* index, const QVariant& data) ;
    /**
      * Correct QVariant types if neccessary because javascript/qt-qml don't have fixed types e.g. int:0 == double:0
      * Return false if not all values can be converted. Return true and change the internal data if successful.
      * @param types Example: ("some_key":"int" , "some_other_key": "string")
      */
    bool correctTypes(const QVariantMap& types);
	
    /***************** Convinience Getter/Setter ******************/
    QString id() const {
        return m_map.value(QLatin1String("id_")).toString();
    }
    // Unique id: type+id
    QString uid() const {
        return m_map.value(QLatin1String("type_")).toString()+m_map.value(QLatin1String("id_")).toString();
    }
    static QString id(const QVariantMap& data) {
        return data.value(QLatin1String("id_")).toString();
    }
    static QString idkey() { return QLatin1String("id_"); }
    void setid(const QString& id) {
      m_map[QLatin1String("id_")] = id;
    }
    bool hasid() const {
        return m_map.contains(QLatin1String("id_"));
    }

    QString pluginid() const {
        return m_map.value(QLatin1String("pluginid_")).toString();
    }
    void setPluginid(const QString& pluginid) {
        m_map[QLatin1String("pluginid_")] = pluginid;
    }
    bool hasPluginid() const {
        return m_map.contains(QLatin1String("pluginid_"));
    }
    bool hasPluginuid() const {
		return m_map.contains(QLatin1String("pluginid_")) && m_map.contains(QLatin1String("plugininstance_"));
	}
    
    QString plugininstance() const{
        return m_map[QLatin1String("plugininstance_")].toString();
    }
    void setPlugininstance(const QString& instanceid) {
        m_map[QLatin1String("plugininstance_")] = instanceid;
    }
    QString pluginuid() const{
        return m_map.value(QLatin1String("pluginid_")).toString()+m_map[QLatin1String("plugininstance_")].toString();
    }
    
    
    QString type() const {
        return m_map.value(QLatin1String("type_")).toString();
    }
    bool hasType() const {
        return m_map.contains(QLatin1String("type_"));
    }
    
    QString sceneid() const {
        return m_map.value(QLatin1String("collection_")).toString();
    }
    void setSceneid(const QString& collectionid) {
        m_map[QLatin1String("collection_")] = collectionid;
    }

    QByteArray configurationkey() const {
        return m_map.value(QLatin1String("configkey_")).toByteArray();
    }
    void setConfigurationkey(const QByteArray& configurationkey) {
        m_map[QLatin1String("configkey_")] = configurationkey;
    }

    int actiondelay() const {
        return m_map.value(QLatin1String("delay_"), 0).toInt();
    }
    
    enum TypeEnum {
        TypeUnknown,
	// Documents that are stored to disk and need to have an ID
        TypeEvent,
        TypeCondition,
        TypeAction,
        TypeScene,
        TypeConfiguration,
        TypeSchema,
	
	// Special types
        TypeExecution, 	// Same as Action but is executed immediately and not stored
        TypeRemove,	// Remove stored document
        TypeNotification,// A notification
        TypeModelItem	// An item of a model
    };

    static QString stringFromTypeEnum(const TypeEnum t) {
	switch (t) {
	  case TypeEvent: return QLatin1String("event");
	  case TypeCondition: return QLatin1String("condition");
	  case TypeAction: return QLatin1String("action");
	  case TypeScene: return QLatin1String("scene");
	  case TypeConfiguration: return QLatin1String("configuration");
	  case TypeSchema: return QLatin1String("schema");
	  default:
	    break;
	};
	return QString();
    }
    
    bool checkType(const TypeEnum t) const {
        const QByteArray type = m_map.value(QLatin1String("type_")).toByteArray();
        return (
			(type == "action" && t==TypeAction) ||
			(type == "condition" && t==TypeCondition) ||
			(type == "event" && t==TypeEvent) ||
			(type == "collection" && t==TypeScene) ||
			(type == "execute" && t==TypeExecution) ||
			(type == "remove" && t==TypeRemove) ||
			(type == "notification" && t==TypeNotification) ||
			(type == "model" && t==TypeModelItem) ||
			(type == "configuration" && t==TypeConfiguration)
		);
    }
    
    QString filename() const {return id() + QLatin1String(".json"); }

    bool isNegatedCondition() const {
        return m_map.value(QLatin1String("conditionnegate_")).toBool();
    }
    QString conditionGroup() const {
        QString cg = m_map.value(QLatin1String("conditiongroup_")).toString();
        if (cg.isEmpty()) cg = QLatin1String("all");
        return cg;
    }


    bool isMethod(const char* id) const {
        return m_map.value(QLatin1String("method_")).toByteArray() == QByteArray(id);
    }
    QByteArray method() const {
        return m_map.value(QLatin1String("method_")).toByteArray();
    }
    void setMethod(const QByteArray& methodname) {
        m_map[QLatin1String("method_")] = methodname;
    }
    bool hasMethod() const {
        return m_map.contains(QLatin1String("method_"));
    }

    int sessionid() const {
        return m_map.value(QLatin1String("sessionid_"),-1).toInt();
    }

    void removeSessionID() {
        m_map.remove(QLatin1String("sessionid_"));
    }

    void setSessionID(const int sessionid) {
        m_map[QLatin1String("sessionid_")] = sessionid;
    }

    bool isQtSlotRespons() const {
        return m_map.contains(QLatin1String("isrespons_"));
    }

    void setQtSlotRespons() {
        m_map[QLatin1String("isrespons_")] = true;
    }
};