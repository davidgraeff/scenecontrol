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

#include "eventdatetime.h"
#include <profile/serviceproviderprofile.h>
#include <RoomControlClient.h>

EventDateTime::EventDateTime(QObject* parent)
        : AbstractEvent(parent)
{
}

QString EventDateTime::datetime() const
{
    return m_datetime.toString(Qt::ISODate);
}

void EventDateTime::setDatetime(QString value)
{
    m_datetime = QDateTime::fromString(value, Qt::ISODate);
}
void EventDateTime::changed() {
    m_string = tr("Auslösen am %1 um %2").arg(m_datetime.date().toString(QLatin1String("dd.mm.yy")))
               .arg(m_datetime.time().toString(QLatin1String("hh:mm")));
    AbstractServiceProvider::changed();
}

void EventDateTime::link() {
    AbstractEvent::link();
    ProfileCollection* c = qobject_cast<ProfileCollection*>(RoomControlClient::getFactory()->get(parentid()));
    if (!c) return;
    RoomControlClient::getProfilesWithAlarmsModel()->addedProvider(c);
}
QDateTime EventDateTime::datetimeRaw() const {
    return m_datetime;
}
