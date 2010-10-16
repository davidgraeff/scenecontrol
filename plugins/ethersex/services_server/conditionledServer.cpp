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

#include "conditionled.h"
#include <RoomControlServer.h>
#include <ledcontroller.h>

ConditionLed::ConditionLed(QObject* parent)
: AbstractCondition(parent)
{  
}

bool ConditionLed::ok()
{
  const unsigned int v = RoomControlServer::getLedController()->getChannel(m_channel);
  if (v>m_max) return false;
  if (v<m_min) return false;
  return true;
}

unsigned int ConditionLed::channel() const
{
    return m_channel;
}

void ConditionLed::setChannel(unsigned int value)
{
    m_channel = value;
}

unsigned int ConditionLed::min() const
{
    return m_min;
}

void ConditionLed::setMin(unsigned int value)
{
    m_min = value;
}

unsigned int ConditionLed::max() const
{
    return m_max;
}

void ConditionLed::setMax(unsigned int value)
{
    m_max = value;
}