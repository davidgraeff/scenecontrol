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
#include <QVariantMap>
#include <QTimer>

#undef PLUGIN_ID
#define PLUGIN_ID "CollectionController"
#include "shared/abstractserver_collectioncontroller.h"
#include <shared/abstractplugin_services.h>
#include <shared/abstractplugin.h>

class PluginController;

class RunningCollection: public QObject {
    Q_OBJECT
public:
    RunningCollection(const QVariantList& actions, const QString& collectionid);
    void start();
private:
    QString m_collectionid;
    QTimer m_timer;
    int m_lasttime;
    QMultiMap<int, QVariantMap> m_timetable;
private Q_SLOTS:
  void timeout();
Q_SIGNALS:
    void runningCollectionAction ( const QVariantMap& actiondata );
    void runningCollectionFinished (const QString& collectionid);
};

class CollectionController: public QObject, public AbstractServerCollectionController, public AbstractPlugin, public AbstractPlugin_services {
    Q_OBJECT
    PLUGIN_MACRO
public:
    CollectionController ();
    virtual ~CollectionController();
    void setPluginController ( PluginController* pc );
private:
    PluginController* m_plugincontroller;
    QMap<QString, RunningCollection*> m_runningCollections;
    void updateListOfRunningCollections();

    /////////////// server interface ///////////////
    virtual void pluginEventTriggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid = "" );
    virtual void pluginRequestExecution ( const QVariantMap& data, const char* pluginid = "" );

    // satisfy interfaces
    virtual void clear() {}
    virtual void initialize() {}
    virtual bool condition(const QVariantMap&, int) {
        return false;
    }
    virtual void execute(const QVariantMap&, int); // implement execute of interface AbstractPlugin_services
    virtual void register_event(const QVariantMap&, const QString&, int) {}
    virtual void unregister_event(const QString&, int) {}
    virtual QList< QVariantMap > properties(int) {
        return QList< QVariantMap >();
    }
public Q_SLOTS:
    void requestExecution ( const QVariantMap& data, int sessionid);
    void runningCollectionAction ( const QVariantMap& actiondata );
    void actionsOfCollection ( const QVariantList& actions, const QString& collectionid);
    void runningCollectionFinished (const QString& collectionid);
};
