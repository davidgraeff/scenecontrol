/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2012  David Gräff

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
#include <QTimer>
#include <QList>
#include "shared/jsondocuments/scenedocument.h"
#include <QDir>
#include <QStringList>
#include <QSet>
#include <qmutex.h>

class DataStorageWatcher;
class AbstractStorageNotifierInterface;

/**
 * Access to documents. It provides signals for changed or removed documents and
 * allow StorageNotifiers to be registered to also get this information.
 * Manages a memory cache for fast access of event, condition, action, scene, configurations
 * documents.
 */
class DataStorage: public QObject {
    Q_OBJECT
public:
	virtual ~DataStorage();
    static DataStorage* instance();
	/**
	 * Utility method: Build a list of subdirectories of the given dir with absolute paths.
	 */
    static QStringList directories(const QDir& dir);
	/**
	 * Return the directory that all documents were loaded from.
	 */
    QDir datadir() const;

    /**
     * Load event, condition, action, scene, configuration documents into the memory cache
     * and build index structures.
     */
    void load();

    /**
      * Unload the cache, remove index structures and StorageNotifier Objects
      */
    void unload();
	
    /**
     * Request all documents of a type. This will use the cache so
     * only types within the cache are allowed (event, condition, action, scene, configuration).
     */
    QList<SceneDocument*> filteredDocuments(SceneDocument::TypeEnum type, const QVariantMap& filter = QVariantMap()) const;
	SceneDocument* getDocument(const QString& uid);
	/**
	 * Thread safe method. Return an invalid SceneDocument if the requested one does not exist
	 */
	SceneDocument getDocumentCopy(const QString& uid);

    /**
     * Request all documents from the disk. Does not use the cache.
     */
    void fetchAllDocuments( QList< SceneDocument >& result ) const;
	
	/**
	 * Register a notifier Object. Whenever storage documents are removed, created or edited
	 * all registered StorageNotifier Objects will be notified.
	 * Registered notifiers are removed automatically if they are deleted.
	 */
	void registerNotifier(AbstractStorageNotifierInterface* notifier);
	/**
	 * This method should not be called manually. It will be invoked
	 * if a registered StorageNotifier Object gets deleted and will remove its reference
	 * from m_notifiers.
	 */
	public Q_SLOTS: void unregisterNotifier(QObject * obj);
public:
    /**
	 * Remove a document permanently from disc
	 */
    bool removeDocument(const SceneDocument& doc);

    /**
      * Store a document given by data on disk.
      * @param doc Have to be valid. Refer to the documentation.
      */
    SceneDocument* storeDocument( const SceneDocument& odoc, bool overwriteExisting = false );

    /**
     * Change only one value of one or more documents (synchronous)
	 * Specifiy documents by the type and filter arguments.
	 * Set "value" to the "key" property of all matched documents.
	 * Return number of changed documents.
     */
    int changeDocumentsValue(SceneDocument::TypeEnum type, const QVariantMap& filter, const QString& key, const QVariantMap& value);
    
    /**
      * Return true if document with type and id is already stored
      */
    bool contains(const SceneDocument &doc) const;
	/**
	 * Create a scene item and add it to the scene
	 * @param scene_uid The scene have to exist
	 */
	void createSceneItem(const QString& scene_id, const SceneDocument& sceneitem);
	/**
	 * Remove a scene item from an existing scene and remove the sceneitem document.
	 * The scene has to be disabled from execution before calling this method
	 * for the case that the to-be-removed scene item is an event.
	 * @param scene_uid The scene have to exist
	 * @param sceneitem_uid The scene item have to exist
	 */
	void removeSceneItem(const QString& scene_id, const QString& sceneitem_uid);
private Q_SLOTS:
	SceneDocument* reloadDocument(const QString& filename);
	void removeFromCache(const QString &filename);
private:
    DataStorage ();
    QDir m_dir;
	QMutex mReadWriteLockMutex;
    DataStorageWatcher* m_listener;
    // data and indexes
    QMap<SceneDocument::TypeEnum, QList<SceneDocument*>> m_cache;
    QMap<QString, SceneDocument*> m_index_typeid;
	QMap<QString, SceneDocument*> m_index_filename;
	
	QSet<AbstractStorageNotifierInterface*> m_notifiers;
    bool m_isLoading;
    
    QList<SceneDocument*> filterEntries(const QList< SceneDocument* >& source, const QVariantMap& filter = QVariantMap()) const;
	QString storagePath(const SceneDocument& doc);
	
	bool removeNotExistingSceneItems(SceneDocument* scene);
};

/**
 * AbstractStorageNotifier
 */
class AbstractStorageNotifierInterface: public QObject {
	friend class DataStorage;
private:
	// Called by the DataStorage
	virtual void documentChanged(const QString& filename, SceneDocument* oldDoc, SceneDocument* newDoc) = 0;
	// Called by the DataStorage
	virtual void documentRemoved(const QString& filename, SceneDocument* document) = 0;
};
