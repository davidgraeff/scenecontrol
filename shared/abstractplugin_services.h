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

/**
 * Plugin interface for dealing with actions, conditions, events and properties
 */
class AbstractPlugin_services
{
public:
    /**
     * Return current state of all plugin properties. The server
     * reguests all properties from all plugins after a client has connected.
     * Example in the VariantMap: fancyplugin_ledIsOn = true
     * Note: Properties are temporary and are not saved by the server.
     * You should cache longer-to-generate properties. This call
     * should not block the server noticable!
     * \param sessionid id of the client session that requests properties of this plugin
     */
    virtual QList<QVariantMap> properties(int sessionid) = 0;
    /**
     * Implement execution routines for all provided actions
     */
    virtual void execute(const QVariantMap& data, int sessionid) = 0;

    /**
    * Implement check routines for all provided conditions
    */
    virtual bool condition(const QVariantMap& data, int sessionid) = 0;

    /**
    * Collections may register events here. Events are triggered by plugins with event_trigger.
    */
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) = 0;

    /**
    * When collections get removed they unregister their events with this method. If collections are changed
    * they will call unregister_event and register_event in sequence.
    */
    virtual void unregister_event ( const QString& eventid, int sessionid ) = 0;
};
Q_DECLARE_INTERFACE(AbstractPlugin_services, "com.roomcontrol.PluginServices/2.0")
