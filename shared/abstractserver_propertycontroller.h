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

class AbstractServerPropertyController
{
public:
    /**
     * A plugin state/property has changed
     * \param data values of the property with special entry id (unqiue identifier within current plugin properties)
     * \param sessionid if not empty, the server will propagate this property change only to the client with this sessionid
     */
    virtual void pluginPropertyChanged(const QVariantMap& data, int sessionid = -1, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Register a listener for a property. The server will send all changes of thsi property back to the plugin via otherPropertyChanged
     */
    virtual void pluginRegisterPropertyChangeListener(const QString& unqiue_property_id, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Unregister a listener. No more changes are propagated for this specific property.
     */
    virtual void pluginUnregisterPropertyChangeListener(const QString& unqiue_property_id, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Unregister all listeners. otherPropertyChanged will not be called by the server anymore.
     */
    virtual void pluginUnregisterAllPropertyChangeListeners(const char* pluginid = PLUGIN_ID) = 0;
};
