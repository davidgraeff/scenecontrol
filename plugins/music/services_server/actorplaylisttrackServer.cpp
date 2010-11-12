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

#include "actorplaylisttrackServer.h"
#include <services/actorplaylisttrack.h>
#include <statetracker/mediastatetracker.h>
#include "playlistServer.h"
#include "services/playlist.h"
#include "server/plugin_server.h"
#include "server/mediacontroller.h"

void ActorPlaylistTrackServer::execute()
{
    MediaController* mc = m_plugin->mediacontroller();
    ActorPlaylistTrack* base = service<ActorPlaylistTrack>();
    // set playlist
    if (base->playlistid().size())
    {
      mc->setPlaylistByID(base->playlistid());
    }
    // set track number
    if (base->track() != -1)
    {
        ActorPlaylistServer* p = mc->playlist();
        if (!p) return;
        p->playlist()->setCurrentTrack(base->track());
    }

    if (base->state() == MediaStateTracker::PlayState)
    {
	mc->play();
    } else if (base->state() == MediaStateTracker::PauseState)
    {
        mc->pause();
    } else if (base->state() == MediaStateTracker::StopState)
    {
        mc->stop();
    }
}

ActorPlaylistTrackServer::ActorPlaylistTrackServer(ActorPlaylistTrack* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}
