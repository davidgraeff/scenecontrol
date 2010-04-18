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

#include "actorprojector.h"
#include <RoomControlClient.h>

ActorProjector::ActorProjector(QObject* parent)
        :AbstractActor(parent)
{
}

void ActorProjector::setCmd(int v) {
    m_cmd = v;
}

int ActorProjector::cmd() {
    return m_cmd;
}

void ActorProjector::changed()
{
    switch (m_cmd) {
		case ProjectorOn: m_string = tr("Projektor an"); break;
		case ProjectorOff: m_string = tr("Projektor aus"); break;
		case ProjectorVideoMute: m_string = tr("Schwarzbild"); break;
		case ProjectorVideoUnMute: m_string = tr("Bild anzeigen"); break;
		case ProjectorLampNormal: m_string = tr("Lampe: Normal"); break;
		case ProjectorLampEco: m_string = tr("Lampe: Eco"); break;
		default: {}
	}
    AbstractActor::changed();
}
