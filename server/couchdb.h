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
    void start();
    void requestActionsOfCollection(const QString& collecion_id);
private:
    CouchDB ();
    int m_last_changes_seq_nr;
    bool checkFailure(QNetworkReply*);
private Q_SLOTS:
    void replyEventsChange();
    void replyActionOfCollection();
    void replyEvent();
    void replyEvents();
    void replyDatabaseInfo();
    void error(QNetworkReply::NetworkError);
Q_SIGNALS:
    void couchDB_failed(const QString& url);
    void couchDB_ready();
    void couchDB_Event_add(const QString& id, const QVariantMap& event_data);
    void couchDB_Event_remove(const QString& id);
    void couchDB_actionsOfCollection(const QVariantList& actions, const QString& collectionid);
};
