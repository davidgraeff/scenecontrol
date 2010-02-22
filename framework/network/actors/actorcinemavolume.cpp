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

#include "actorcinemavolume.h"

ActorCinemaVolume::ActorCinemaVolume(QObject* parent)
        : AbstractActor(parent)
{}

qreal ActorCinemaVolume::value() const {
    return m_volume;
}
void ActorCinemaVolume::setValue(qreal value) {
    m_volume = value;
}
bool ActorCinemaVolume::relative() const {
    return m_relative;
}
void ActorCinemaVolume::setRelative(bool value) {
    m_relative = value;
}
void ActorCinemaVolume::changed() {
    if (m_relative)
        m_string = tr("Cinema Volume verändern um %1").arg(m_volume);
    else
        m_string = tr("Cinema Volume setzten auf %1").arg(m_volume);
    AbstractServiceProvider::changed();
}

