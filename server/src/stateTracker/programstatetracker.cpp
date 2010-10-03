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

#include "programstatetracker.h"
#include "networkcontroller.h"
#include <RoomControlServer.h>
#include <factory.h>
#include <abstractserviceprovider.h>
#include <QCoreApplication>

ProgramStateTracker::ProgramStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{}

QString ProgramStateTracker::appversion() const {
    return QCoreApplication::applicationVersion();
}

QString ProgramStateTracker::minversion() const {
    return QLatin1String(NETWORK_MIN_APIVERSION);
}

QString ProgramStateTracker::maxversion() const {
    return QLatin1String(NETWORK_MAX_APIVERSION);
}

QStringList ProgramStateTracker::serviceprovider() {
    QStringList l;
    QList<AbstractServiceProvider*> asplist = RoomControlServer::getFactory()->m_providerList;
    foreach (AbstractServiceProvider* p, asplist)
    {
        l.append(p->id());
    }
    return l;
}

QStringList ProgramStateTracker::statetracker() {
    QStringList l;
    QList<AbstractStateTracker*> asplist = RoomControlServer::getRoomControlServer()->getStateTracker();
    foreach (AbstractStateTracker* p, asplist)
    {
        l.append(p->type());
    }
    return l;
}
