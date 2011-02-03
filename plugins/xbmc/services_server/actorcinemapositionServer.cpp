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

#include "actorcinemapositionServer.h"
#include <services/actorcinemaposition.h>
#include "server/plugin_server.h"

void ActorCinemaPositionServer::execute()
{
    m_plugin->setVolume(service<ActorCinemaPosition>()->value(),service<ActorCinemaPosition>()->relative());
}
ActorCinemaPositionServer::ActorCinemaPositionServer(ActorCinemaPosition* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}
void ActorCinemaPositionServer::nameUpdate() {
	ActorCinemaPosition* base = service<ActorCinemaPosition>();
	
	base->setString(tr("XBMC Position: %1").arg(base->value()));
}