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
        : AbstractEvent(parent), m_aftertrigger(false)
{
    m_timer.setSingleShot(true);
    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
    for (int i=0;i<7;++i)
        m_days.append(false);
}

void EventPeriodic::nextWeekday(int &day)
{
    ++day;
    if ( day > 6 )
        day = 0;
}

void EventPeriodic::newvalues()
{
    timeout(true);
	AbstractEvent::newvalues();
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

void EventPeriodic::timeout(bool aftersync)
{
    // timer triggered this timeout
    if (!aftersync && !m_aftertrigger)  {
        emit eventTriggered();
		// get back in 2 seconds to generate the new periodic alarm
		m_aftertrigger = true;
		m_timer.start(2000);
		return;
	}
	m_aftertrigger = false;

    if ( !m_time.isValid() ) return;

    QDateTime datetime = QDateTime::currentDateTime();
    datetime.setTime ( m_time );
    int dow = QDate::currentDate().dayOfWeek() - 1;
    int offsetdays = 0;

    // If it is too late for the alarm today then
    // try with the next weekday
    if ( QTime::currentTime() > m_time )
    {
        nextWeekday(dow);
        ++offsetdays;
    }

    // Search for the next activated weekday
    for (; offsetdays < 8; ++offsetdays )
    {
        if ( m_days[dow] ) break;
        nextWeekday(dow);
    }

    if ( offsetdays < 8 )
    {
        const int sec = QDateTime::currentDateTime().secsTo (datetime.addDays ( offsetdays ));
        m_timer.start(sec*1000);
		qDebug() << "Periodic alarm: Armed" << sec;
    }
}
