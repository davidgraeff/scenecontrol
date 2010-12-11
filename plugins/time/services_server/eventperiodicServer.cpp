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

#include "eventperiodicServer.h"
#include <QDebug>
#include <services/eventperiodic.h>
#include <QDateTime>
#include "server/plugin_server.h"

void EventPeriodicServer::timeout(bool aftersync)
{
    EventPeriodic* base = service<EventPeriodic>();
    QTime m_time = QDateTime::fromString(base->time(),Qt::ISODate).time();
    // timer triggered this timeout
    if (!aftersync && !m_aftertrigger)  {
        emit trigger();
        // get back in 2 seconds to generate the new periodic alarm
        m_aftertrigger = true;
        m_timer.start(2000);
        return;
    }
    m_aftertrigger = false;

    QDateTime datetime = QDateTime::currentDateTime();
    datetime.setTime ( m_time );
    int dow = QDate::currentDate().dayOfWeek() - 1;
    int offsetdays = 0;

    // If it is too late for the alarm today then
    // try with the next weekday
    if ( QTime::currentTime() > m_time )
    {
        dow = (dow+1) % 7;
        ++offsetdays;
    }

    // Search for the next activated weekday
    for (; offsetdays < 8; ++offsetdays )
    {
        if ( base->m_days[dow] ) break;
        dow = (dow+1) % 7;
    }

    if ( offsetdays >= 8 ) {
        return;
		qWarning()<<"Periodic alarm: Failure with weekday offset"<<offsetdays<<datetime.toString(Qt::DefaultLocaleShortDate);
    }

    const int sec = QDateTime::currentDateTime().secsTo (datetime.addDays ( offsetdays ));
    m_timer.start(sec*1000);
	qDebug() << "Periodic alarm: " << datetime.addDays ( offsetdays ).toString(Qt::DefaultLocaleShortDate);
}
EventPeriodicServer::EventPeriodicServer(EventPeriodic* base, myPluginExecute*, QObject* parent) : ExecuteService(base, parent), m_aftertrigger(false)
{
    m_timer.setSingleShot(true);
    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}
bool EventPeriodicServer::checkcondition() {
    return true;
}
void EventPeriodicServer::dataUpdate() {
    timeout(true);
}
void EventPeriodicServer::execute() {}

void EventPeriodicServer::nameUpdate() {
    EventPeriodic* base = service<EventPeriodic>();
    QString days;
    for (int i=0;i<7;++i) {
        if (base->days() & (1<<i)) {
            days.append(QDate::shortDayName(i+1));
            days.append(QLatin1String(","));
        }
    }
    days.chop(1);
    base->setString(tr("Periodisch um %1\nan: %2").arg(base->time()).arg(days));
}
