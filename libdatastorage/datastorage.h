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

class DataStorageWatcher;
class AbstractStorageNotifier;

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
public Q_SLOTS:
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
    QList<SceneDocument*> requestAllOfType(SceneDocument::TypeEnum type, const QVariantMap& filter = QVariantMap()) const;

    /**
     * Request all documents from the disk. Does not use the cache.
     */
    const QList< SceneDocument >& fetchAllDocuments() const;
	
	/**
	 * Register a notifier Object. Whenever storage documents are removed, created or edited
	 * all registered StorageNotifier Objects will be notified.
	 */
	void registerNotifier(AbstractStorageNotifier* notifier);
    
    /// Remove a document
    void removeDocument(const SceneDocument &doc);

    /**
      * Store a document given by data on disk.
      * @param doc Have to be valid. Refer to the documentation.
      */
    bool storeDocument( const SceneDocument& doc, bool overwriteExisting = false );

    /**
     * Change configurations of a given plugin (synchronous)
     * \param category An arbitrary non empty word describing the configuration category of the values
     */
    int changeDocumentsValue(SceneDocument::TypeEnum type, const QVariantMap& filter, const QString& key, const QVariantMap& value);
    
    /**
      * Return true if document with type and id is already stored
      */
    bool contains(const SceneDocument &doc) const;
private Q_SLOTS:
	void reloadDocument(const QString &filename);
	void removeFromCache(const QString &filename);
	/**
	 * This is not a public method and should not be called manually. It will be invoked
	 * if a registered StorageNotifier Object gets deleted and will remove its reference
	 * from m_notifiers.
	 */
	void unregisterNotifier(QObject * obj);
private:
    DataStorage ();
    QDir m_dir;
    DataStorageWatcher* m_listener;
    // data and indexes
    QMap<QString, QList<SceneDocument*>> m_cache;
    QMap<QString, SceneDocument*> m_index_typeid;
	QMap<QString, SceneDocument*> m_index_filename;
	
	QSet<AbstractStorageNotifier*> m_notifiers;
    
    QList<SceneDocument*> filterEntries(const QList< SceneDocument* >& source, const QVariantMap& filter = QVariantMap()) const;

Q_SIGNALS:
	/// A document changed: only triggered if startChangeListener has been called
    void doc_changed(const SceneDocument*);
	/// A document has been removed: only triggered if startChangeListener has been called
    void doc_removed(const SceneDocument*);
};

/**
 * AbstractStorageNotifier
 */
class AbstractStorageNotifier: public QObject {
	friend class DataStorage;
private:
	// Called by the DataStorage
	virtual void documentChanged(const QString& filename, SceneDocument* document) = 0;
	// Called by the DataStorage
	virtual void documentRemoved(const QString& filename, SceneDocument* document) = 0;
};
