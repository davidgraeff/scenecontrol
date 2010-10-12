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

#include "eventdatetime.h"
#include <QDebug>

EventDateTime::EventDateTime(QObject* parent)
        : AbstractEvent(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

void EventDateTime::newvalues()
{
    timeout();
    AbstractEvent::newvalues();
}

QString EventDateTime::datetime() const
{
    return m_datetime.toString(Qt::ISODate);
}

void EventDateTime::setDatetime(QString value)
{
    m_datetime = QDateTime::fromString(value, Qt::ISODate);
}

void EventDateTime::timeout()
{
    const int sec = QDateTime::currentDateTime().secsTo (m_datetime);

    if (sec > 86400) {
        qDebug() << "One-time alarm: Armed (next check only)";
        m_timer.start(86400*1000);
    } else if (sec > 10) {
		qDebug() << "One-time alarm: Armed" << sec;
        m_timer.start(sec*1000);
    } else if (sec > -10 && sec < 10) {
        emit eventTriggered();
    }
}
