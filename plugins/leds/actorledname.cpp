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

#include "actorledname.h"
#include <RoomControlServer.h>
#include <ledcontroller.h>

ActorLedName::ActorLedName(QObject* parent)
        : AbstractActor(parent)
{}

void ActorLedName::execute()
{
    RoomControlServer::getLedController()->setChannelName(m_channel,m_ledname);
}

unsigned int ActorLedName::channel() const
{
    return m_channel;
}

void ActorLedName::setChannel(unsigned int value)
{
    m_channel = value;
}

QString ActorLedName::ledname() const
{
    return m_ledname;
}

void ActorLedName::setLedname(const QString& ledname)
{
    m_ledname = ledname;
}
