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
#define PLUGIN_ID "PropertyController"
#include "shared/abstractserver_propertycontroller.h"
#include <shared/abstractplugin_services.h>
#include <shared/abstractplugin.h>
#include <QSet>

class WebSocket;
class AbstractPlugin_services;
class QNetworkAccessManager;
class QNetworkReply;
class CollectionInstance;
class Collections;
class PluginController;

class PropertyController: public QObject, public AbstractServerPropertyController, public AbstractPlugin, public AbstractPlugin_services {
    Q_OBJECT
    PLUGIN_MACRO
public:
    PropertyController ();
    virtual ~PropertyController();
    void setPluginController ( PluginController* pc );
    QByteArray allProperties(int sessionid);
private:
    PluginController* m_plugincontroller;
    QMap<QString, QSet<QString> > m_propertyid_to_plugins;

    /////////////// server interface ///////////////
    virtual void pluginPropertyChanged ( const QVariantMap& data, int sessionid = -1, const char* pluginid = "" );
    virtual void pluginRegisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid = "" );
    virtual void pluginUnregisterAllPropertyChangeListeners ( const char* pluginid = "" );
    virtual void pluginUnregisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid = "" );

    // satisfy interfaces
    virtual void clear() {}
    virtual void initialize() {}
    virtual bool condition(const QVariantMap&, int) {
        return false;
    }
    virtual void execute(const QVariantMap&, int); // implement execute of interface AbstractPlugin_services
    virtual void register_event(const QVariantMap&, const QString&, int) {}
    virtual void unregister_event(const QString&, int) {}
    virtual void settingsChanged(const QVariantMap&) {}
    virtual QList< QVariantMap > properties(int);
    virtual void saveSettings(const QString& key, const QVariantMap& value, const char* pluginid);
};
