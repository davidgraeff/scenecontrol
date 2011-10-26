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

class AbstractServerCollectionController
{
public:
    /**
     * Ask the server to execute the action described by the QVariantMap.
     */
    virtual void pluginRequestExecution(const QVariantMap& data, const char* pluginid = PLUGIN_ID) = 0;
    /**
     * Plugin event triggered
     * \param event_id unqiue id (guid) of the triggered event
     * \param destination_collectionuid only send this event trigger to the collection with this uid. Event trigger will be discarded if empty!
     */
    virtual void pluginEventTriggered(const QString& event_id, const QString& destination_collectionuid, const char* pluginid = PLUGIN_ID) = 0;
};
