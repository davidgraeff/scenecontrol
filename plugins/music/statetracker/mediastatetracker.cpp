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

#include "mediastatetracker.h"
#include <RoomControlServer.h>
#include <media/mediacontroller.h>
#include <media/playlist.h>

MediaStateTracker::MediaStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{
}

QString MediaStateTracker::playlistid() const
{
    Playlist* p = RoomControlServer::getMediaController()->playlist();
    if (!p) return QString();
    return p->id();
}

qint64 MediaStateTracker::position() const
{
    return RoomControlServer::getMediaController()->getTrackPosition();
}

qint64 MediaStateTracker::total() const
{
    return RoomControlServer::getMediaController()->getTrackDuration();
}

int MediaStateTracker::state()
{
    return RoomControlServer::getMediaController()->state();
}

int MediaStateTracker::track() const
{
    Playlist* p = RoomControlServer::getMediaController()->playlist();
    if (!p) return -1;
    return p->currentTrack();
}

