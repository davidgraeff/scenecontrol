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

#include "mediastatetracker.h"

MediaStateTracker::MediaStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{
}

const QString& MediaStateTracker::playlistid() const {
    return m_playlistid;
}
void MediaStateTracker::setPlaylistid(const QString& p) {
    m_playlistid = p;
}
qint64 MediaStateTracker::position() const {
    return m_position;
}
void MediaStateTracker::setPosition(qint64 p) {
    m_position = p;
}
qint64 MediaStateTracker::total() const {
    return m_total;
}
void MediaStateTracker::setTotal(qint64 t) {
    m_total = t;
}
qreal MediaStateTracker::volume() const {
    return m_volume;
}
void MediaStateTracker::setVolume(qreal v) {
    m_volume = v;
}
int MediaStateTracker::state() const {
    return m_state;
}
void MediaStateTracker::setState(int s) {
    m_state = s;
}
int MediaStateTracker::track() const {
    return m_track;
}
void MediaStateTracker::setTrack(int t) {
    m_track = t;
}

