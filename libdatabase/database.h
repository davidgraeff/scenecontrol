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
#include "mongo/client/dbclient.h"

class Database: public QObject {
    Q_OBJECT
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
public:
    /**
     * Database is a singleton object
     */
    static Database* instance();
    virtual ~Database();
	QString databaseAddress() const;
	enum ConnectStateEnum {
		DisconnectedState = 0,
		ConnectingState = 1,
		ConnectedState = 2,
		ConnectedButNotInialized = 3
	};
	ConnectStateEnum state() {return m_state;}

public Q_SLOTS:
    /**
     * Connect to database (synchronous)
     * Return true if the connection could be established
     */
    ConnectStateEnum connectToDatabase(const QString& serverHostname, bool reconnectOnFailure);

	/**
	 * Remove all listeners and disconnect from database
	 */
	void disconnectFromHost();
	
    /**
     * Request all events (asynchronous)
     * Event_add and Event_remove signals are triggered in reaction
     */
    void requestEvents(const QString& plugin_id, const QString& instanceid);
	
    /**
     * Request all conditions and actions of a collection (asynchronous)
     * dataOfCollection signal is triggered in reaction
     */
    void requestDataOfCollection(const QString& collection_id);
	
    /**
     * Request all configurations of a given plugin (asynchronous)
     * settings signal is triggered in reaction
     */
    void requestPluginConfiguration(const QString& pluginid);
	
    /**
     * Change configurations of a given plugin (synchronous)
     * settings signal is triggered in reaction if startChangeListenerSettings has been called before
	 * \param category An arbitrary non empty word describing the configuration category of the values
     */
    void changePluginConfiguration(const QString& pluginid, const QString& instanceid, const QByteArray& category, const QVariantMap& value);
	
    /**
     * Export all json documents to the given path (synchronous)
     */
    void exportAsJSON(const QString& path);
    /**
     * Import all json documents from the given path recursively (synchronous)
     */
    void importFromJSON(const QString& path);

    /**
     * Request all collections (asynchronous)
     * doc_changed signal is triggered in reaction
     */
    void requestCollections();
	
    /**
     * Request all schemas (asynchronous)
     * doc_changed signal is triggered in reaction
     */
    void requestSchemas();
	
	/// Remove a document
    void removeDocument(const QString &type, const QString& id);

	/**
	 * Change or insert document given by data.
	 * data have to contain a field "_id" if insertWithNewID is not true
	 */
	bool changeDocument(const QVariantMap& data, bool insertWithNewID = false);
	/**
	 * Return true if document with type and id is already stored
	 */
	bool contains(const QString &type, const QString& id);
private:
    Database ();
    QString m_serveraddress;
	/// current state
	ConnectStateEnum m_state;
    bool m_reconnectOnFailure;
	void changeState(ConnectStateEnum newstate);
	QTimer m_reconnectTimer;
	
	mongo::DBClientConnection m_mongodb;
private Q_SLOTS:
	ConnectStateEnum reconnectToDatabase();
Q_SIGNALS:
    /// A configuration either changed or was actively requested
    void pluginConfiguration(const QString& pluginid, const QVariantMap& data);
	/// An event document changed or requestEvents was called before
    void Event_add(const QString& id, const QVariantMap& event_data);
	/// An event document has been removed. This is only triggered if startChangeListenerEvents
	/// has been called before
    void Event_remove(const QString& id);
	/// dataOfCollection is triggered if requestDataOfCollection has been called before
    void dataOfCollection (const QString& collectionid, const QList<QVariantMap>& services);
	/// A document changed: only triggered if startChangeListener has been called
    void doc_changed(const QString& id, const QVariantMap& data);
	/// A document has been removed: only triggered if startChangeListener has been called
    void doc_removed(const QString& id);
	/// Connection state changed
	void stateChanged();
};
