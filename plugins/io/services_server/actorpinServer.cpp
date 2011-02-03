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

#include "actorpinServer.h"
#include <services/actorpin.h>
#include "server/plugin_server.h"
#include "server/iocontroller.h"

void ActorPinServer::execute()
{
  ActorPin* a = service<ActorPin>();
  if (a->value()==ActorPin::PinToggle)
    m_plugin->controller()->togglePin(a->pin());
  else
    m_plugin->controller()->setPin(a->pin(),a->value());
}

ActorPinServer::ActorPinServer(ActorPin* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}

void ActorPinServer::nameUpdate() {
	ActorPin* base = service<ActorPin>();
	
	QString pinname = m_plugin->controller()->getPinName(base->pin());
        if (pinname.isEmpty()) pinname = base->pin();
	
	if (base->value()==ActorPin::PinToggle)
		base->setString(tr("Pin %1\nUmschalten").arg(pinname));
	else if (base->value()==ActorPin::PinOn)
		base->setString(tr("Pin %1\nEinschalten").arg(pinname));
	else if (base->value()==ActorPin::PinOff)
		base->setString(tr("Pin %1\nAusschalten").arg(pinname));
}