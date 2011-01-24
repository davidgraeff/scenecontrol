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

#include "actoreventServer.h"
#include <services/actorevent.h>
#include <server/plugin_server.h>
#include <server/externalclient.h>

ActorEventServer::ActorEventServer(ActorEvent* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}

void ActorEventServer::execute()
{
    ActorEvent* base = service<ActorEvent>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        if (base->filename().size())
            client->playEvent(base->filename());
        if (base->title().size())
            client->showMessage(base->duration(),base->title());
    }
}

bool ActorEventServer::checkcondition() {
    return true;
}
void ActorEventServer::dataUpdate() {}

void ActorEventServer::nameUpdate() {
    ActorEvent* base = service<ActorEvent>();

    base->setString(tr("Ereignis %1 auslösen\n%2").arg(base->title()).arg(base->filename()));
}
