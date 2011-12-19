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

	Purpose: Load plugins, load description xmls, route services and properties
*/

#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QSet>
#include <QVariantMap>
#include <QNetworkReply>

class CouchDB: public QNetworkAccessManager {
    Q_OBJECT
public:
    static CouchDB* instance();
    virtual ~CouchDB();
    bool connectToDatabase();
    void requestActionsOfCollection(const QString& collecion_id);
    void requestPluginSettings(const QString& pluginid, bool tryToInstall = true);
private:
    CouchDB ();
    int m_last_changes_seq_nr;
    bool checkFailure(QNetworkReply*);
    
    int installPluginData(const QString& pluginid);
private Q_SLOTS:
    // Called if events on the database changed and fetches those events
    void replyEventsChange();
    // Called as result of replyEventsChange
    void replyEvent();
    // Receive settings of a plugin. As a respons to the method requestPluginSettings
    void replyPluginSettings();
    // Called if plugin settings on the database changed and fetches those settings. Will fire the signal couchDB_settings
    void replyPluginSettingsChange();
    // Fetches all conditions and actions of a collection. Will fire the signal couchDB_actionsOfCollection
    void replyActionOfCollection();
    void errorWithRecovery(QNetworkReply::NetworkError);
    void errorFatal(QNetworkReply::NetworkError);
    void errorNoSettings();
Q_SIGNALS:
    // Fail signals
    void couchDB_failed(const QString& url);
    void couchDB_no_settings_found(const QString& prefix);
    // Change signals
    void couchDB_settings(const QString& prefix, const QVariantMap& data);
    void couchDB_Event_add(const QString& id, const QVariantMap& event_data);
    void couchDB_Event_remove(const QString& id);
    void couchDB_actionsOfCollection(const QVariantList& actions, const QString& collectionid);
    // Ready signals
    void couchDB_ready();
};
