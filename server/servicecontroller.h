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

#undef PLUGIN_ID
#define PLUGIN_ID ""
#include <shared/abstractserver.h>
#include <shared/abstractplugin_services.h>
#include <QSet>
#undef PLUGIN_ID

class WebSocket;
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

class ServiceController: public QObject, public AbstractServer {
    Q_OBJECT
    friend class PluginController;
public:
    ServiceController ();
    virtual ~ServiceController();
    void setPluginController ( PluginController* pc );
    bool startWatchingCouchDB();
    QByteArray allProperties(int sessionid);
private:
    WebSocket* m_websocket;
    PluginController* m_plugincontroller;
    int m_last_changes_seq_nr;
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
    virtual void pluginEventTriggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid = "" );
    virtual void pluginRequestExecution ( const QVariantMap& data, const char* pluginid = "" );
    virtual void pluginPropertyChanged ( const QVariantMap& data, int sessionid = -1, const char* pluginid = "" );
    virtual void pluginRegisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid = "" );
    virtual void pluginUnregisterAllPropertyChangeListeners ( const char* pluginid = "" );
    virtual void pluginUnregisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid = "" );
public slots:
    void replyEventsChange();
    void networkReply(QNetworkReply*);
    void requestExecution ( const QVariantMap& data, int sessionid);
};
