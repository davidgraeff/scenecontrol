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

#include "actormute.h"
#include <RoomControlServer.h>
#include <media/mediacontroller.h>

ActorMute::ActorMute(QObject* parent)
        : AbstractActor(parent),m_mute(-1),m_volume(-1.0), m_relative(false)
{}

void ActorMute::execute()
{
    if (m_mute==1)
        RoomControlServer::getMediaController()->setPAMute(m_value.toAscii(),1);
    else if (m_mute==0)
        RoomControlServer::getMediaController()->setPAMute(m_value.toAscii(),0);
	else if (m_mute==2)
		RoomControlServer::getMediaController()->togglePAMute(m_value.toAscii());

	if (m_volume>=0.0)
		RoomControlServer::getMediaController()->setPAVolume(m_value.toAscii(),m_volume,m_relative);
}
