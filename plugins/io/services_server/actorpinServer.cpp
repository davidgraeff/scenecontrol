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

#include "actorpin.h"
#include <RoomControlServer.h>
#include <iocontroller.h>

ActorPin::ActorPin(QObject* parent)
: AbstractActor(parent)
{}

void ActorPin::execute()
{
  if (m_value==PinToggle)
    RoomControlServer::getIOController()->togglePin(m_pin);
  else
    RoomControlServer::getIOController()->setPin(m_pin,m_value);
}

unsigned int ActorPin::pin() const
{
    return m_pin;
}

void ActorPin::setPin(unsigned int value)
{
    m_pin = value;
}

int ActorPin::value() const
{
    return m_value;
}

void ActorPin::setValue(int value)
{
    m_value = value;
}