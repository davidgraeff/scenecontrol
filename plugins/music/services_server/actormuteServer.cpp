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

#include "actormuteServer.h"
#include <services/actormute.h>
#include <plugin_server.h>
#include "../mediacontroller.h"

void ActorMuteServer::execute()
{
  ActorMute* base = service<ActorMute>();
    if (base->mute()==1)
        m_plugin->mediacontroller()->setPAMute(base->value().toAscii(),1);
    else if (base->mute())
        m_plugin->mediacontroller()->setPAMute(base->value().toAscii(),0);
    else if (base->mute())
        m_plugin->mediacontroller()->togglePAMute(base->value().toAscii());

    if (base->volume()>=0.0)
        m_plugin->mediacontroller()->setPAVolume(base->value().toAscii(),base->volume(),base->relative());
}

ActorMuteServer::ActorMuteServer(ActorMute* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}
