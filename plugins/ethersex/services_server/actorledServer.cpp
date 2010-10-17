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

#include "actorledServer.h"
#include <services/actorled.h>
#include <plugin_server.h>
#include <controller.h>

void ActorLedServer::execute()
{
  ActorLed* a = service<ActorLed>();
  if (a->assignment() == ActorLed::ValueAbsolute)
    m_plugin->controller()->setChannel ( a->channel(),a->value(),a->fadetype() );
  else if (a->assignment() == ActorLed::ValueRelative)
    m_plugin->controller()->setChannelRelative ( a->channel(),a->value(),a->fadetype() );
  else if (a->assignment() == ActorLed::ValueMultiplikator)
    m_plugin->controller()->setChannelExponential ( a->channel(),a->value(),a->fadetype() );
  else if (a->assignment() == ActorLed::ValueInverse)
    m_plugin->controller()->inverseChannel ( a->channel(),a->fadetype() );
}


ActorLedServer::ActorLedServer(ActorLed* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}
