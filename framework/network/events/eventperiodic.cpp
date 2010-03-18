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

#include "eventperiodic.h"
#include <profile/serviceproviderprofile.h>
#include <RoomControlClient.h>

EventPeriodic::EventPeriodic(QObject* parent)
        : AbstractEvent(parent)
{
    for (int i=0;i<7;++i)
        m_days.append(false);
}

void EventPeriodic::nextWeekday(int &day)
{
    ++day;
    if ( day > 6 )
        day = 0;
}

QString EventPeriodic::time() const
{
    return m_time.toString(Qt::ISODate);
}

void EventPeriodic::setTime(QString value)
{
    m_time = QTime::fromString(value, Qt::ISODate);
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
void EventPeriodic::changed() {
    QString days;
    for (int i=0;i<m_days.size();++i) {
        if (m_days[i])
            days += QDate::shortDayName(i+1) + QLatin1String(",");
    }
    days.chop(1);
    m_string = tr("%1h am %2").arg(m_time.toString(QLatin1String("hh:mm"))).arg(days);
    AbstractServiceProvider::changed();
}

void EventPeriodic::link() {
    AbstractEvent::link();
    ProfileCollection* c = qobject_cast<ProfileCollection*>(RoomControlClient::getFactory()->get(parentid()));
    if (!c) return;
    RoomControlClient::getProfilesWithAlarmsModel()->addedProvider(c);
}
