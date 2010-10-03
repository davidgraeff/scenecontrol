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

#include "remotecontrolstatetracker.h"
#include <RoomControlServer.h>
#include <remotecontrol/remotecontroller.h>

RemoteControlStateTracker::RemoteControlStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{}

bool RemoteControlStateTracker::connected() const
{
    return RoomControlServer::getRemoteController()->isConnected();
}

int RemoteControlStateTracker::receivers() const
{
    return RoomControlServer::getRemoteController()->countReceiver();
}

QString RemoteControlStateTracker::mode() const
{
    return RoomControlServer::getRemoteController()->mode();
}
