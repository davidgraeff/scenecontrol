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
#include <QNetworkAccessManager>
#include <QVariantMap>
#include <QNetworkReply>

class Database: public QNetworkAccessManager {
    Q_OBJECT
public:
    /**
     * Database is a singleton object
     */
    static Database* instance();
    virtual ~Database();
    /**
     * Connect to database (synchronous)
     * Return true if the connection could be established
     */
    bool connectToDatabase();
    /**
     * Request all events (synchronous)
     * Event_add and Event_remove signals are triggered in reaction
     */
    void requestEvents(const QString& plugin_id);
    /**
     * Install missing json documents for the given plugin (synchronous)
     */
    bool verifyPluginData(const QString& pluginid);
    /**
     * Request all conditions and actions of a collection (asynchronous)
     * dataOfCollection signal is triggered in reaction
     */
    void requestDataOfCollection(const QString& collection_id);
    /**
     * Request all configurations of a given plugin (synchronous)
     * settings signal is triggered in reaction
     */
    void requestPluginConfiguration(const QString& pluginid);
    /**
     * Change configurations of a given plugin (synchronous)
     * settings signal is triggered in reaction if startChangeListenerSettings has been called before
     */
    void changePluginConfiguration(const QString& pluginid, const QString& key, const QVariantMap& value);
    /**
     * Export all json documents to the given path (synchronous)
     */
    void exportAsJSON(const QString& path);
    /**
     * Import all json documents from the given path recursively (synchronous)
     */
    void importFromJSON(const QString& path);
public Q_SLOTS:
    /**
	 * Start listening to configuration changes (change+remove)
	 * If the database does not respond 5 times in sequal the application loop will be quit.
     * settings signal is triggered in reaction if a change occured
     */
    void startChangeListenerSettings();
    /**
	 * Start listening to event document changes (change+remove)
	 * If the database does not respond 5 times in sequal the application loop will be quit.
     * Event_add and Event_remove signals are triggered in reaction if a change occured
     */
    void startChangeListenerEvents();
private:
    Database ();
    int m_last_changes_seq_nr;
    /// Keep track of the failures. If > 5 exit application because database is down.
    int m_settingsChangeFailCounter;
    /// Keep track of the failures. If > 5 exit application because database is down.
    int m_eventsChangeFailCounter;
    /// Check for the return value of the NetworkReply. Output error message+msg if an error occured.
    bool checkFailure(QNetworkReply* r, const QByteArray& msg=QByteArray());

private Q_SLOTS:
    /// Called if events on the database changed. Fetch all those events
    void replyEventsChange();
    /// Called if plugin configuration in the database changed.
    /// Fetch those new configurations. settings signal is triggered in reaction
    void replyPluginSettingsChange();
    /**
	 * The caller requested a list of all conditions and actions of a collection.
     * Now fetch all those condition and action documents.
     * dataOfCollection signal is triggered in reaction
	 */
    void replyDataOfCollection();
Q_SIGNALS:
    /// A configuration either changed or was actively requested
    void settings(const QString& pluginid, const QString& key, const QVariantMap& data);
	/// An event document changed or requestEvents was called before
    void Event_add(const QString& id, const QVariantMap& event_data);
	/// An event document has been removed. This is only triggered if startChangeListenerEvents
	/// has been called before
    void Event_remove(const QString& id);
	/// dataOfCollection is triggered if requestDataOfCollection has been called before
    void dataOfCollection ( const QList<QVariantMap>& actions, const QList<QVariantMap>& conditions, const QString& collectionid);
    /// Database connection is ready
    void ready();
};
