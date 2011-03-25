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

// access and set unique id (uid)
#define UNIQUEID() data[QLatin1String("__uid")].toString()
#define SETUNIQUEID(VAR) data[QLatin1String("__uid")] = VAR

// contains
#define CONTAINS(ID) data.contains(QLatin1String(ID))

// special attributes
#define IS_TOBEREMOVED() CONTAINS("__remove")
#define IS_TOBEEXECUTED() CONTAINS("__execute")

// service type
#define IS_ACTION() (data[QLatin1String("__type")].toString() == QLatin1String("action"))
#define IS_CONDITION() (data[QLatin1String("__type")].toString() == QLatin1String("condition"))
#define IS_EVENT() (data[QLatin1String("__type")].toString() == QLatin1String("event"))
#define IS_COLLECTION() (data[QLatin1String("__type")].toString() == QLatin1String("collection"))
#define IS_PROPERTY() (data[QLatin1String("__type")].toString() == QLatin1String("property"))

// getter
#define MAP(ID) data[QLatin1String(ID)].toMap()
#define LIST(ID) data[QLatin1String(ID)].toList()
#define DATA(ID) data[QLatin1String(ID)].toString()
#define INTDATA(ID) data[QLatin1String(ID)].toInt()
#define BOOLDATA(ID) data[QLatin1String(ID)].toBool()
#define DOUBLEDATA(ID) data[QLatin1String(ID)].toDouble()
#define PROPERTY(ID) QVariantMap data; data[QLatin1String("id")].toString() = QLatin1String(PLUGIN_ID"_"ID)

// setter
#define SETDATA(ID, DATA) data[QLatin1String(ID)] = DATA

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
    virtual QMap<QString, QVariantMap> properties(const QString& sessionid) = 0;
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
    virtual void event_changed(const QVariantMap& data) = 0;
};
Q_DECLARE_INTERFACE(AbstractPlugin_services, "com.roomcontrol.PluginServices/2.0")
