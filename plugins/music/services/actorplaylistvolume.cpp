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

#include "actorplaylistvolume.h"

ActorPlaylistVolume::ActorPlaylistVolume(QObject* parent)
        : AbstractServiceProvider(parent)
{}

qreal ActorPlaylistVolume::volume() const {
    return m_volume;
}
void ActorPlaylistVolume::setVolume(qreal value) {
    m_volume = value;
}
bool ActorPlaylistVolume::relative() const {
    return m_relative;
}
void ActorPlaylistVolume::setRelative(bool value) {
    m_relative = value;
}

