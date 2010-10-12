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

#include "actorwol.h"
#include <QMetaProperty>
#include <QDebug>
#include <QUdpSocket>

ActorWOL::ActorWOL(QObject* parent)
        : AbstractActor(parent)
{
}

void ActorWOL::execute() {
	QStringList parts = m_mac.split(QLatin1Char(':'));
	if (parts.size()!=6) return;
	QByteArray mac;
	for (int i=0;i<6;++i)
		mac.append(QByteArray::fromHex(parts[i].toAscii()));
	
	// 6 mal FF
	const char header[] = {255,255,255,255,255,255};
	QByteArray bytes(header);
	// 16 mal mac
	for (int i=0;i<16;++i)
		bytes.append(mac);
	
    QUdpSocket s;
	s.writeDatagram(bytes,QHostAddress::Broadcast,9);
}
