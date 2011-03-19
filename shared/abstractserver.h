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

#ifndef PLUGIN_ID
#error Define PLUGIN_ID before including this header!
#endif

class AbstractServer
{
public:
    /**
     * Plugin event triggered
     * \param event_id unqiue id (guid) of the triggered event
     */
    virtual void event_triggered(const QString& event_id, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * A plugin state/property has changed
     * \param value values of the property with special entry id (unqiue identifier within current plugin properties)
     */
    virtual void property_changed(const QVariantMap& values, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Register a listener for a property. The server will send all changes of thsi property back to the plugin via otherPropertyChanged
     */
    virtual void register_listener(const QString& unqiue_property_id, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Unregister a listener. No more changes are propagated for this specific property. 
     */
    virtual void unregister_listener(const QString& unqiue_property_id, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Unregister all listeners. otherPropertyChanged will not be called by the server anymore.
     */
    virtual void unregister_all_listeners(const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Ask the server to execute the action described by the VariantMap. The fields "plugin_id" and "name" are mandatory.
     */
    virtual void execute_action(const QVariantMap& action, const char* pluginid = PLUGIN_ID) = 0;

};
