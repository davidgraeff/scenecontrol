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

#include "actorplaylistcmd.h"

#include "RoomControlServer.h"
#include "media/mediacontroller.h"
#include "media/mediacmds.h"

ActorPlaylistCmd::ActorPlaylistCmd(QObject* parent)
        : AbstractActor(parent)
{
}

void ActorPlaylistCmd::execute()
{
    MediaController* mc = RoomControlServer::getMediaController();
    if (m_cmd == PlayCmd)
    {
        mc->play();
    } else if (m_cmd == PauseCmd)
    {
        mc->pause();
    } else if (m_cmd == StopCmd)
    {
        mc->stop();
    } else if (m_cmd == NextCmd)
    {
        mc->next();
    } else if (m_cmd == PrevCmd)
    {
        mc->previous();
    } else if (m_cmd == NextPlaylistCmd)
    {
        mc->nextPlaylist();
    } else if (m_cmd == PrevPlaylistCmd)
    {
        mc->previousPlaylist();
    }
}
int ActorPlaylistCmd::cmd() const {
    return m_cmd;
}
void ActorPlaylistCmd::setCmd(int value) {
    m_cmd = value;
}
