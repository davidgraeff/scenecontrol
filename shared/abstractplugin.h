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
#include <QSet>

#define PLUGIN_MACRO \
protected: \
	virtual QString pluginid() { return QLatin1String(PLUGIN_ID); } \
	AbstractServer* m_server; \
public: \
	virtual void connectToServer(AbstractServer* server) {m_server=server; } \

// access and set id
#define IS_ID(ID) data[QLatin1String("id")].toString() == QLatin1String(ID)
#define ID() data[QLatin1String("id")].toString()
#define SETID(VAR) data[QLatin1String("id")] = VAR

class AbstractServer;
class AbstractPlugin
{
public:
    /**
     * Hint: Do not reimplement this method (It is implemented within the PLUGIN_MACRO macro).
     */
	virtual QString pluginid() = 0;
	
    /**
	 * Hint: Do not reimplement this method (It is implemented within the PLUGIN_MACRO macro).
     */
    virtual void connectToServer(AbstractServer* server) = 0;

    /**
     * (Re)Initialize the plugin. Called after all plugins are loaded but before the
     * network is initiated or by request from a client with sufficient access rights.
     */
	virtual void initialize() = 0;
	
    /**
     * Called by server process before it releases all ressources and finish.
     * Tidy up here.
     */
    virtual void clear() = 0;
	
};
Q_DECLARE_INTERFACE(AbstractPlugin, "com.roomcontrol.Plugin/2.0")
