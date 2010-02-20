/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "actorplaylisttrack.h"
#include "media/mediacmds.h"

ActorPlaylistTrack::ActorPlaylistTrack(QObject* parent)
        : AbstractActor(parent)
{
}

QString ActorPlaylistTrack::playlistid() const {
    return m_playlistid;
}
void ActorPlaylistTrack::setPlaylistID(QString playlistid) {
    m_playlistid = playlistid;
}

int ActorPlaylistTrack::track() const {
    return m_track;
}
void ActorPlaylistTrack::setTrack(int value) {
    m_track = value;
}

int ActorPlaylistTrack::state() const {
    return m_state;
}
void ActorPlaylistTrack::setState(int value) {
    m_state = value;
}
void ActorPlaylistTrack::changed() {
    m_string = tr("Musik von %1 abspielen").arg(m_playlistid);
    AbstractServiceProvider::changed();
}
