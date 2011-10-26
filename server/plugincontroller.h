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
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>
#include <shared/abstractplugin_sessions.h>

#undef PLUGIN_ID
#define PLUGIN_ID "PluginController"

class CollectionController;
class PropertyController;

class PluginInfo {
public:
    AbstractPlugin* plugin;
    QString version;
    void setVersion(const QString& version) {
        this->version = version;
    }
    PluginInfo(AbstractPlugin* plugin) {
        this->plugin=plugin;
    }
    ~PluginInfo() {/* do not delete plugin. will be done by QPluginLoader automaticly */ }
};

class PluginController: public QObject, public AbstractPlugin, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
public:
    PluginController (PropertyController* propertycontroller, CollectionController* collectioncontroller);
    virtual ~PluginController();
    void initializePlugins();
    int knownServices();

    AbstractPlugin* getPlugin(const QString& pluginid);

    QMap<QString,PluginInfo*>::iterator getPluginIterator();
    AbstractPlugin* nextPlugin(QMap<QString,PluginInfo*>::iterator& index);
	AbstractPlugin_settings* nextSettingsPlugin(QMap<QString,PluginInfo*>::iterator& index);
    AbstractPlugin_services* nextServicePlugin(QMap<QString,PluginInfo*>::iterator& index);
	AbstractPlugin_sessions* nextSessionPlugin(QMap<QString,PluginInfo*>::iterator& index);

    // satisfy interfaces
    virtual void clear(){}
    virtual void initialize(){}
    virtual bool condition(const QVariantMap&, int){return false;}
    virtual void execute(const QVariantMap&, int) {}
    virtual void register_event(const QVariantMap&, const QString&, int){}
    virtual void unregister_event(const QString&, int){}

    // Properties
    virtual QList< QVariantMap > properties(int sessionid);
private:
    QMap<QString,PluginInfo*> m_plugins;
    int m_index;
    QMap<QString, AbstractPlugin_services*> m_registeredevents;
public Q_SLOTS:
    void couchDB_Event_add(const QString& id, const QVariantMap& event_data);
    void couchDB_Event_remove(const QString& id);
};
#undef PLUGIN_ID