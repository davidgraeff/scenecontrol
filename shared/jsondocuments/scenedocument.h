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
#include <QStringList>

/**
 * A scene document is the in-memory copy of a json document, technically implemented
 * with a QVariantMap. This class provides a lot of convenience functions to access typical
 * scene document properties like "id_", "type_" or "componentid_".
 */
class SceneDocument {
public:
    /** 
     * Constructor: Construct by an existing QVariantMap
     */
    SceneDocument(const QVariantMap& map);
	SceneDocument(const QVariant& v = QVariant());
    /** 
     * Constructor: Construct by a json document
     */
    SceneDocument(const QByteArray& jsondata);

	
	enum TypeEnum {
		TypeUnknown,
		// Documents that are stored to disk and need to have an ID
		TypeEvent,TypeCondition,TypeAction,TypeScene,
		TypeConfiguration,TypeSchema,
		
		// Special types
		TypeExecution, 	// Same as Action but is executed immediately and not stored
		TypeRemove,	// Remove stored document
		TypeNotification,// A notification
		
		TypeModelItem, TypeModelItemRemove,TypeModelItemChange,TypeModelItemReset,
		
		TypeError,
		
		TypeLAST
	};
	
	static QString typeString(const TypeEnum t);
	
	bool isType(const TypeEnum t) const {
		return (t==mType);
	}
	bool isOneOfType(const TypeEnum t[], int typeArraySize) const {
		for(int i=0;i<typeArraySize;++i)
			if (t[i]==mType) return true;
			return false;
	}
	
    
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

	void checkIfIDneedsGUID();
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
    bool correctDataTypes(const QVariantMap& types);
	
    /***************** Convenience Getter/Setter ******************/
    QString id() const {
        return m_map.value(QLatin1String("id_")).toString();
    }
    // Unique id: type+id
    QString uid() const {
        return m_map.value(QLatin1String("type_")).toString()+m_map.value(QLatin1String("id_")).toString();
    }
    
    static QString uid(SceneDocument::TypeEnum type, const QString& id)
	{
		return typeString(type)+id;
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

    QString componentID() const {
        return m_map.value(QLatin1String("componentid_")).toString();
    }
    void setComponentID(const QString& pluginid) {
        m_map[QLatin1String("componentid_")] = pluginid;
    }
    bool hasComponentID() const {
		return m_map.contains(QLatin1String("componentid_"));
	}
    bool hasComponentUniqueID() const {
		return m_map.contains(QLatin1String("componentid_")) && m_map.contains(QLatin1String("instanceid_"));
	}
    QString componentUniqueID() const{
        return m_map.value(QLatin1String("componentid_")).toString()+m_map[QLatin1String("instanceid_")].toString();
    }
    
    QString instanceID() const{
        return m_map[QLatin1String("instanceid_")].toString();
    }
    void setInstanceID(const QString& instanceid) {
        m_map[QLatin1String("instanceid_")] = instanceid;
    }

    
    void setType(TypeEnum t) {
		mType = t;
		m_map[QLatin1String("type_")] = QByteArray(typetext[t]);
	}
	TypeEnum type() const {
		return mType;
    }
    bool hasType() const {
        return mType!=TypeUnknown;
    }
    
    QString sceneid() const {
        return m_map.value(QLatin1String("sceneid_")).toString();
    }
    void setSceneid(const QString& collectionid) {
        m_map[QLatin1String("sceneid_")] = collectionid;
    }

    QByteArray modelkey() const {
        return m_map.value(QLatin1String("key_")).toByteArray();
    }
    void setModelkey(const QByteArray& configurationkey) {
        m_map[QLatin1String("key_")] = configurationkey;
    }

    QString filename() const {return id() + QLatin1String(".json"); }

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

    // User session specific
    int sessionid() const {
        return m_map.value(QLatin1String("sessionid_"),-1).toInt();
    }

    void removeSessionID() {
        m_map.remove(QLatin1String("sessionid_"));
    }

    void setSessionID(const int sessionid) {
        m_map[QLatin1String("sessionid_")] = sessionid;
    }
    
    // Scene specific
    QVariantList nextNodes() const {
		return m_map.value(QLatin1String("e")).toList();
	}
	void setNextNodes(const QVariantList& nextNodes) {
		m_map[QLatin1String("e")] = nextNodes;
	}
	QVariantList nextAlternativeNodes() const {
		return m_map.value(QLatin1String("eAlt")).toList();
	}
	void setAlternativeNextNodes(const QVariantList& nextNodes) {
		m_map[QLatin1String("eAlt")] = nextNodes;
	}
	QVariantList sceneItems() const {
		return m_map.value(QLatin1String("v")).toList();
	}
	
private:
	QVariantMap m_map;
	static const char* const typetext[];
	TypeEnum mType;
	void convertType();
};

