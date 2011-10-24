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
#include <QMap>
#include <QVariantMap>
#include <QTimer>

#ifndef PLUGIN_ID
#define PLUGIN_ID ""
#endif

#include <shared/abstractserver.h>
#include <QSet>
#include <QSocketNotifier>

class libwebsocket;
class libwebsocket_context;
class AbstractPlugin_services;
class QNetworkAccessManager;
class QNetworkReply;
class CollectionInstance;
class Collections;
class PluginController;

struct TriggerChange {
	bool deleted;
	QString id;
	QString collectionid;
};

class ServiceController: public QObject, public AbstractServer{
    Q_OBJECT
public:
    ServiceController ();
    virtual ~ServiceController();
    void setPluginController ( PluginController* pc );
    bool startWatchingCouchDB();
    void websocketClientRequestAllProperties(struct libwebsocket *wsi);
    void addWebsocketFD(int fd, short int direction);
    void removeWebsocketFD(int fd);
private:
    PluginController* m_plugincontroller;
    int m_last_changes_seq_nr;
    struct libwebsocket_context* m_websocket_context;
    QMap<int, QSocketNotifier*> m_websocket_fds;
	QNetworkAccessManager *m_manager;
	QSet<QNetworkReply*> m_eventreplies;
	QSet<QNetworkReply*> m_executecollection;
	QSet<QNetworkReply*> m_actionreplies;
    QMap<QString, QPair<QVariantMap,AbstractPlugin_services*> > m_registeredevents;

	void requestDatabaseInfo();
	void requestEvents();
	void startChangeLister();
	void registerEvent(const QVariantMap& data);
    // routing
    QMap<QString, QSet<QString> > m_propertyid_to_plugins;

    /////////////// server interface ///////////////
    virtual void event_triggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid = "" );
    virtual void execute_action ( const QVariantMap& data, const char* pluginid = "" );
    virtual void property_changed ( const QVariantMap& data, const QString& sessionid = QString(), const char* pluginid = "" );
    virtual void register_listener ( const QString& unqiue_property_id, const char* pluginid = "" );
    virtual void unregister_all_listeners ( const char* pluginid = "" );
    virtual void unregister_listener ( const QString& unqiue_property_id, const char* pluginid = "" );
	
public slots:
    void replyEventsChange();
    void networkReply(QNetworkReply*);
    void websocketactivity(int);
};
