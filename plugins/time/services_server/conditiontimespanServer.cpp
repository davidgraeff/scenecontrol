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

#include "conditiontimespanServer.h"
#include <QDateTime>
#include <services/conditiontimespan.h>
#include <plugin_server.h>


bool ConditionTimespanServer::checkcondition() {
  const ConditionTimespan* base = (ConditionTimespan*)baseService();
  QTime m_lower = QTime::fromString(base->lower(),Qt::ISODate);
  QTime m_upper = QTime::fromString(base->upper(),Qt::ISODate);
    if (QTime::currentTime() < m_lower) return false;
    if (QTime::currentTime() > m_upper) return false;
    return true;
}
void ConditionTimespanServer::dataUpdate() {}
void ConditionTimespanServer::execute() {}
ConditionTimespanServer::ConditionTimespanServer(ConditionTimespan* base, myPluginExecute* , QObject* parent) : ExecuteService(base, parent) {}
