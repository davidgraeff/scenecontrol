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

#include "actorplaylistcmdServer.h"
#include <services/actormusiccmd.h>
#include "server/plugin_server.h"
#include "server/mediacontroller.h"

ActorPlaylistCmdServer::ActorPlaylistCmdServer(ActorPlaylistCmd* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}

void ActorPlaylistCmdServer::execute()
{
    ActorPlaylistCmd* base = service<ActorPlaylistCmd>();
    MediaController* mc = m_plugin->mediacontroller();
    if (base->cmd() == ActorPlaylistCmd::PlayCmd)
    {
        mc->play();
    } else if (base->cmd() == ActorPlaylistCmd::PauseCmd)
    {
        mc->pause();
    } else if (base->cmd() == ActorPlaylistCmd::StopCmd)
    {
        mc->stop();
    } else if (base->cmd() == ActorPlaylistCmd::NextCmd)
    {
        mc->next();
    } else if (base->cmd() == ActorPlaylistCmd::PrevCmd)
    {
        mc->previous();
    } else if (base->cmd() == ActorPlaylistCmd::NextPlaylistCmd)
    {
        mc->nextPlaylist();
    } else if (base->cmd() == ActorPlaylistCmd::PrevPlaylistCmd)
    {
        mc->previousPlaylist();
    } else if (base->cmd() == ActorPlaylistCmd::DumpMediaInfoCmd)
    {
        mc->dumpMediaInfo();
    }
}
void ActorPlaylistCmdServer::nameUpdate() {
	ActorPlaylistCmd* base = service<ActorPlaylistCmd>();
	
	base->setString(tr("MPD Kommando: %1").arg(base->translate(0,base->cmd())));
}
