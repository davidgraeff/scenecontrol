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

#include "eventperiodic.h"
#include <QDebug>

EventPeriodic::EventPeriodic(QObject* parent)
        : AbstractServiceProvider(parent)
{
      for (int i=0;i<7;++i)
        m_days[i] = false;
}

QString EventPeriodic::time() const
{
    return m_time;
}

void EventPeriodic::setTime(QString value)
{
    m_time = value;
}

int EventPeriodic::days() const
{
    int t = 0;
    for (int i=0;i<7;++i)
        if (m_days[i])
            t |= (1 << i);
    return t;
}

void EventPeriodic::setDays(int value)
{
    for (int i=0;i<7;++i)
        m_days[i] = (value & (1 << i));
}
