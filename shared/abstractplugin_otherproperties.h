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

class AbstractPlugin_otherproperties
{
public:
    /**
     * Called by server process if a state of a property of another plugin or a client property changed.
     * This plugin has to register its interest in one or more properties to the server by
     * using the AbstractServer Interface before.
     * \param data the property data with special entry id (unqiue identifier within current plugin properties)
	 * \param sessionid If sessionid is not null this property is a session propery and received from a client and only valid for the referred session
     */
    virtual void otherPropertyChanged(const QVariantMap& data, const QString& sessionid) = 0;
};
Q_DECLARE_INTERFACE(AbstractPlugin_otherproperties, "com.roomcontrol.PluginOtherProperties/2.0")
