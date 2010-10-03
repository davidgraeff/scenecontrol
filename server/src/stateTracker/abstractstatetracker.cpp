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

#include "abstractstatetracker.h"
#include <QUuid>
#include "RoomControlServer.h"
#include "networkcontroller.h"
#include "factory.h"

AbstractStateTracker::AbstractStateTracker(QObject* parent)
: QObject(parent) {
    NetworkController* nc = RoomControlServer::getNetworkController();
    connect(this,SIGNAL(objectSync(QObject*)), nc,
            SLOT(objectSync(QObject*)));
}

AbstractStateTracker::~AbstractStateTracker()
{
    setProperty("remove",true);
    emit objectSync(this);
}

void AbstractStateTracker::sync()
{
    emit objectSync(this);
}

QString AbstractStateTracker::type() const {
    const QString type = QString::fromAscii(metaObject()->className());
    return type;
}
