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

#include "actorled.h"

ActorLed::ActorLed(QObject* parent)
        : AbstractServiceProvider(parent)
{}

unsigned int ActorLed::channel() const
{
    return m_channel;
}

void ActorLed::setChannel(unsigned int value)
{
    m_channel = value;
}

ActorLed::fadetypeEnum ActorLed::fadetype() const
{
    return m_fadetype;
}

void ActorLed::setFadetype(ActorLed::fadetypeEnum value)
{
    m_fadetype = value;
}

int ActorLed::value() const
{
    return m_value;
}

void ActorLed::setValue(int value)
{
    m_value = value;
}

ActorLed::assignmentEnum ActorLed::assignment() const
{
    return m_assignment;
}

void ActorLed::setAssignment(ActorLed::assignmentEnum value)
{
    m_assignment = value;
}
