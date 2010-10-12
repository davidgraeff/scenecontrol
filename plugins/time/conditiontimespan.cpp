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

#include "conditiontimespan.h"
#include <RoomControlServer.h>

ConditionTimespan::ConditionTimespan(QObject* parent)
: AbstractCondition(parent)
{  
}


bool ConditionTimespan::ok()
{
  if (QTime::currentTime() < m_lower) return false;
  if (QTime::currentTime() > m_upper) return false;
  return true;
}

QString ConditionTimespan::lower() const
{
    return m_lower.toString(Qt::ISODate);
}

void ConditionTimespan::setLower(QString value)
{
    m_lower = QTime::fromString(value, Qt::ISODate);
}

QString ConditionTimespan::upper() const
{
    return m_upper.toString(Qt::ISODate);
}

void ConditionTimespan::setUpper(QString value)
{
    m_upper = QTime::fromString(value, Qt::ISODate);
}
