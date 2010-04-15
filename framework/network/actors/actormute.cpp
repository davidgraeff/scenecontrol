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
        : AbstractActor(parent),m_mute(-1),m_volume(-1.0), m_relative(false)
{}

void ActorMute::changed() {
		m_string = tr("PA Sink %1: ").arg(m_value);
    if (m_mute==1)
        m_string += tr("Mute");
    else if (m_mute==0)
        m_string += tr("Unmute");
    else if (m_mute==2)
        m_string += tr("Toggle mute");
	else if (m_volume>=0.0)
		m_string += tr("Volume %1").arg(m_volume);
    AbstractActor::changed();
}
