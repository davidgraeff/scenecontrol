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
#include <qpointer.h>

class PluginCommunication;
class PluginController;

class RunningCollection: public QObject {
    Q_OBJECT
public:
    RunningCollection(const QList<QVariantMap>& actions, const QList<QVariantMap>& conditions, const QString& collectionid);
    void start();
private:
    QString m_collectionid;
    QTimer m_timer;
    int m_lasttime;
    struct dataWithPlugin {
        QPointer<PluginCommunication> plugin;
        QVariantMap data;
        dataWithPlugin(QPointer<PluginCommunication> p, QVariantMap m) : plugin(p), data(m) {}
    };
    QMultiMap<int, dataWithPlugin> m_timetable;
    QList<dataWithPlugin> m_conditions;
private Q_SLOTS:
    void timeout();
Q_SIGNALS:
    void runningCollectionFinished (const QString& collectionid);
};

class CollectionController: public QObject {
    Q_OBJECT

private:
    CollectionController ();
public:
    static CollectionController* instance();
    virtual ~CollectionController();
private:
    PluginController* m_plugincontroller;
    QMap<QString, RunningCollection*> m_runningCollections;
    void updateListOfRunningCollections();
public Q_SLOTS:
    void requestExecutionByCollectionId ( const QString& collectionid );
    void requestExecution ( const QVariantMap& data, int sessionid );
    void runningCollectionFinished (const QString& collectionid);
    void dataOfCollection ( const QList<QVariantMap>& actions, const QList<QVariantMap>& conditions, const QString& collectionid);
};
