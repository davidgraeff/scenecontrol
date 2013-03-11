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
#include <QUuid>

/**
 * A scene document is the in-memory copy of a json document, technically implemented
 * with a QVariantMap. This class provides a lot of convenience functions to access typical
 * scene document properties like "id_", "type_" or "componentid_".
 */
class SceneDocument {
public:
	enum TypeEnum {
		TypeUnknown,
		// Documents that are stored to disk and need to have an ID
		TypeEvent,TypeCondition,TypeAction,TypeScene,
		TypeConfiguration,TypeSchema,
		
		// Special types
		TypeExecution, 	// Same as Action but is executed immediately and not stored
		TypeRemove,	// Remove stored document
		TypeNotification,// A notification
		
		TypeModelItem,
		
		TypeError, TypeAck, TypeAuth,
		
		TypeLAST
	};
	
	/***************** Constructor and hash ******************/
	/** 
     * Constructor: Construct by an existing QVariantMap
     */
    SceneDocument(const QVariantMap& map);
	SceneDocument() {}
    /** 
     * Constructor: Construct by a json document
     */
    SceneDocument(const QByteArray& jsondata, const QByteArray& hash);
	SceneDocument(const QByteArray& jsondata);
	
	bool isResponse(SceneDocument& doc) {return requestid()==doc.responseid();}

	const QByteArray getHash() ;
	QString filename() const ;
	QString responseid() const {return m_map.value(QLatin1String("responseid_")).toString();}
	QString requestid() const {return m_map.value(QLatin1String("requestid_")).toString();}
	void setrequestid() {m_map[QLatin1String("requestid_")] = QUuid::createUuid().toString();}
	void makeack(const QString& requestid) {setType(TypeAck); m_map[QLatin1String("responseid_")] = requestid;}
	
	/***************** For Model messages/Notification ******************/
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

	QByteArray modelkey() const ;
	void setModelkey(const QByteArray& configurationkey) ;
	
    /***************** Is valid ******************/
	void checkIfIDneedsGUID();
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
	
    /***************** Convenience: Access ID + UID ******************/
    QString id() const ;
	void setid(const QString& id) ;
	bool hasid() const ;
	QString uid() const ; // Unique id: type+id
	static QString uid(SceneDocument::TypeEnum type, const QString& id);
    static QString id(const QVariantMap& data) ;
    static QString idkey() ;

	/***************** Convenience: Access componentID + instanceID ******************/
	QString componentID() const ;
    void setComponentID(const QString& pluginid) ;
    bool hasComponentID() const ;
    bool hasComponentUniqueID() const ;
    QString componentUniqueID() const;
    
    QString instanceID() const;
    void setInstanceID(const QString& instanceid) ;

	/***************** Convenience: Access type ******************/
    void setType(TypeEnum t) ;
	TypeEnum type() const ;
    bool hasType() const ;
	static QString typeString(const TypeEnum t);
	bool isType(const TypeEnum t) const ;
	bool isOneOfType(int typeArraySize, ...) const ;
	
	/***************** Convenience: Access sceneID (for scene items) ******************/
    QString sceneid() const ;
    void setSceneid(const QString& collectionid) ;

	/***************** Convenience: Access method ******************/
	bool isMethod(const char* id) const ;
    QByteArray method() const ;
    void setMethod(const QByteArray& methodname) ;
    bool hasMethod() const ;

	/***************** Convenience: User session specific ******************/
    int sessionid() const ;
    void removeSessionID() ;
    void setSessionID(const int sessionid) ;
    
	/***************** Convenience: Scene specific ******************/
    QVariantList nextNodes() const ;
	void setNextNodes(const QVariantList& nextNodes) ;
	QVariantList nextAlternativeNodes() const ;
	void setAlternativeNextNodes(const QVariantList& nextNodes) ;
	QVariantList sceneItems() const ;
	void removeSceneItem(SceneDocument* sceneItemDoc);
	void addSceneItem(SceneDocument* sceneItemDoc);
	void setSceneItems(const QVariantList& sceneitemList);
	
private:
	QVariantMap m_map;
	static const char* const typetext[];
	TypeEnum mType;
	QByteArray mHash;
	void convertType();
};

