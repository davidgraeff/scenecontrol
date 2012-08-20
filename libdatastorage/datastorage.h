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

	Purpose: Database convenience singleton object for
	(1) importing json documents into the database (importFromJSON)
	(1) exporting json documents out of the database (exportAsJSON)
	(1) installing plugin json documents in the database if not already installed (verifyPluginData)
	(2) listening for event document changes (startChangeListenerEvents)
	(2) request for all events of a plugin (requestEvents)
	(3) listening for configuration document changes (startChangeListenerSettings)
	(3) requesting configuration for a plugin (requestPluginSettings)
*/

#pragma once
#include <QVariantMap>
#include <QTimer>
#include <QList>
#include "shared/jsondocuments/scenedocument.h"
#include <QDir>
#include <QStringList>

class DataStorageWatcher;
class DataStorage: public QObject {
    Q_OBJECT
public:
	virtual ~DataStorage();
    static DataStorage* instance();
public Q_SLOTS:
    QDir datadir() const;
    static QStringList directories(const QDir& dir);

    /**
     * Connect to database (synchronous)
     * Return true if the connection could be established
     */
    void load();

    /**
      * Remove all listeners and disconnect from database
      */
    void unload();
	
    /**
     * Request all documents of a type
     * Event_add and Event_remove signals are triggered in reaction
     */
    QList<SceneDocument*> requestAllOfType(SceneDocument::TypeEnum type, const QVariantMap& filter = QVariantMap()) const;

    /// Remove a document
    void removeDocument(const SceneDocument &doc);

    /**
      * Store a document given by data.
      * data have to contain an ID (id_) field if insertWithNewID is not true
      */
    bool storeDocument(const SceneDocument& doc, bool overwriteExisting = false, bool updateCache = false);

    /**
     * Change configurations of a given plugin (synchronous)
     * settings signal is triggered in reaction if startChangeListenerSettings has been called before
	 * \param category An arbitrary non empty word describing the configuration category of the values
     */
    int changeDocumentsValue(SceneDocument::TypeEnum type, const QVariantMap& filter, const QString& key, const QVariantMap& value);
    
    /**
      * Return true if document with type and id is already stored
      */
    bool contains(const SceneDocument &doc) const;
public Q_SLOTS:
    void updateCache(SceneDocument* doc);
    void removeFromCache(const QString &type, const QString& id);
private:
    DataStorage ();
    QDir m_dir;
    DataStorageWatcher* m_listener;
    // data and indexes
    QMap<QString, QList<SceneDocument*>> m_cache;
    QMap<QString, SceneDocument*> m_index_typeid;
    
    QList<SceneDocument*> filterEntries(const QList< SceneDocument* >& source, const QVariantMap& filter = QVariantMap()) const;
Q_SIGNALS:
	/// A document changed: only triggered if startChangeListener has been called
    void doc_changed(const SceneDocument &doc);
	/// A document has been removed: only triggered if startChangeListener has been called
    void doc_removed(const SceneDocument &doc);
};
