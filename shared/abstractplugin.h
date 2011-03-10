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

*/

#pragma once
#include <QVariantMap>
#include <QString>
#include "abstractserver.h"

class AbstractPlugin
{
public:
    /**
     * (Re)Initialize the plugin. Called after all plugins are loaded but before the
     * network is initiated or by request from a client with sufficient access rights.
     */
    virtual void init(AbstractServer*) = 0;
    /**
     * Return current state of all plugin properties. The server
     * reguests all properties from all plugins after a client has connected.
     * Example in the VariantMap: fancyplugin_ledIsOn = true
     * Note: Properties are temporary and are not saved by the server.
     * Cache properties from devices that need a while for responses. This call
     * should not block the server for a longer period!
     */
    virtual QVariantMap properties() = 0;
    /**
     * Called by server process before it releases all ressources and finish.
     * Tidy up here.
     */
    virtual void clear() = 0;
    /**
     * Called by server process if a state of a property of another plugin changed.
     * This plugin has to register its interest in one or more properties to the server by
     * using the AbstractServer Interface before.
     * \param unqiue_property_id the property id (unique among all plugins)
     * \param value the property value
     */
    void otherPropertyChanged(const QString& unqiue_property_id, const QVariant& value) = 0;
    /**
     * Called by server process if a setting is changed. Use the plugin helper file
     * and call setSettings to save settings permanantly.
     * \param name Name of the setting
     * \param value Value of the setting
     */
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false) = 0;
    /**
     * Called by an autogenerated file from cmake build process and registers/sets initial values
     * for settings. Use the plugin helper file and its implementation of registerSettings
     * to (1) use environment variables, (2) load saved settings or use the inital value.
     * \param name Name of the setting
     * \param value Value of the setting
     */
    virtual void registerSetting(const char* name, const QVariant& value) = 0;
    /**
     * Called by the server if a client requested settings of this plugin.
     */
    const QVariantMap getSettings() const = 0;
    
    /**
     * Implement execution routines for all provided actions
     */
    virtual void execute(const QVariantMap& data) = 0;

     /**
     * Implement check routines for all provided conditions
     */
    virtual bool condition(const QVariantMap& data) = 0;

     /**
     * Event data loaded by the server or changed by a client. Implement routines to trigger the actual event based on these data
     * and remove the event with the event id (from data) that was established by previous data.
     */
    virtual bool event_changed(const QVariantMap& data) = 0;

};
Q_DECLARE_INTERFACE(AbstractPlugin, "com.roomcontrol.Plugin/2.0")
