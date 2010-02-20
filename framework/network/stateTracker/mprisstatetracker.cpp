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

#include "mprisstatetracker.h"

MprisStateTracker::MprisStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{
}

void MprisStateTracker::setState(int s) {
    m_state = s;
}
int MprisStateTracker::state() const {
    return m_state;
}
void MprisStateTracker::setVolume(int v) {
    m_volume = v;
}
void MprisStateTracker::setPosition(int p) {
    m_position = p;
}
int MprisStateTracker::volume() const {
    return m_volume;
}
int MprisStateTracker::position() const {
    return m_position;
}
void MprisStateTracker::setUrl(const QString& u) {
    m_url = u;
}
const QString& MprisStateTracker::url() const {
    return m_url;
}
void MprisStateTracker::setMprisid(const QString& m) {
    m_mprisid = m;
}
const QString& MprisStateTracker::mprisid() const {
    return m_mprisid;
}
