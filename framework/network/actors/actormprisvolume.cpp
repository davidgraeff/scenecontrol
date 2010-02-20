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

#include "actormprisvolume.h"

ActorMprisVolume::ActorMprisVolume(QObject* parent)
        : AbstractActor(parent)
{}

qreal ActorMprisVolume::value() const {
    return m_volume;
}
void ActorMprisVolume::setValue(qreal value) {
    m_volume = value;
}
QString ActorMprisVolume::mprisid() const {
    return m_mprisid;
}
void ActorMprisVolume::setMprisid(const QString& value) {
    m_mprisid = value;
}
bool ActorMprisVolume::relative() const {
    return m_relative;
}
void ActorMprisVolume::setRelative(bool value) {
    m_relative = value;
}
void ActorMprisVolume::changed() {
    if (m_relative)
        m_string = tr("Mpris Volume verändern um %1").arg(m_volume);
    else
        m_string = tr("Mpris Volume setzten auf %1").arg(m_volume);
    AbstractServiceProvider::changed();
}

