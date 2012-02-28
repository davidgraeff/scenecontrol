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
#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QDir>
#include <QLocalSocket>
#include <QLocalServer>

class PluginProcess;
class CollectionController;
class PluginCommunication;

class PluginController: public QObject
{
    Q_OBJECT
public:
    static PluginController* instance();
    virtual ~PluginController();
    bool startplugins();
    void unloadPlugin(const QString& id);
    void removePluginFromPending(PluginCommunication* pluginprocess);
    void addPlugin(const QString& id, PluginCommunication* pluginprocess);
    void removeProcess(PluginProcess* process);

    PluginCommunication* getPlugin(const QString& pluginid);

    QMap<QString,PluginCommunication*>::iterator getPluginIterator();
    PluginCommunication* nextPlugin(QMap<QString,PluginCommunication*>::iterator& index);

    void requestAllProperties(int sessionid = -1);
private:
    PluginController ();
    QMap<QString,PluginCommunication*> m_plugins;
    QMap<QLocalSocket*,PluginCommunication*> m_pendingplugins;
    QSet<PluginProcess*> m_pluginprocesses;
    QMap<QString, PluginCommunication*> m_registeredevents;
    int m_index;
    QLocalServer m_comserver;
private Q_SLOTS:
    void newConnection();
public Q_SLOTS:
    void Event_add(const QString& id, const QVariantMap& event_data);
    void Event_remove(const QString& id);
    void failed(const QString& url);
    void settings(const QString& pluginid, const QString& key, const QVariantMap& data);
};
#undef PLUGIN_ID
