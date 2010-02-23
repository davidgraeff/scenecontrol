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

#include "actormute.h"

ActorMute::ActorMute(QObject* parent)
        : AbstractActor(parent)
{}

qreal ActorMute::value() const {
    return m_value;
}
void ActorMute::setValue(qreal value) {
    m_value = value;
}

void ActorMute::changed() {
    if (m_mute)
        m_string = tr("Mute Pulseaudio Sink %1").arg(m_value);
    else
        m_string = tr("Unmute Pulseaudio Sink %1").arg(m_value);
    AbstractActor::changed();
}
