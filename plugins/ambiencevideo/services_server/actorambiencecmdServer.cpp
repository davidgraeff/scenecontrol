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

#include "actorambiencecmdServer.h"
#include "server/plugin_server.h"
#include "server/externalclient.h"
#include <services/actorambiencecmd.h>
#include <shared/abstractplugin.h>

void ActorAmbienceCmdServer::execute()
{
    ActorAmbienceCmd* base = service<ActorAmbienceCmd>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        switch (base->cmd()) {
        case ActorAmbienceCmd::CloseFullscreen:
            client->closeFullscreen();
            break;
        case ActorAmbienceCmd::ScreenOff:
            client->setDisplayState(0);
            break;
        case ActorAmbienceCmd::ScreenOn:
            client->setDisplayState(1);
            break;
        case ActorAmbienceCmd::ScreenToggle:
            client->setDisplayState(2);
            break;
        case ActorAmbienceCmd::HideVideo:
            client->hideVideo();
            break;
        case ActorAmbienceCmd::StopVideo:
            client->stopvideo();
            break;
        default:
            break;
        }
    }
}

ActorAmbienceCmdServer::ActorAmbienceCmdServer(ActorAmbienceCmd* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}

void ActorAmbienceCmdServer::nameUpdate() {
    ActorAmbienceCmd* base = service<ActorAmbienceCmd>();

    base->setString(tr("%1 Kommando\n%2").arg(m_plugin->base()->name()).arg(base->translate(0,base->cmd())));
}
