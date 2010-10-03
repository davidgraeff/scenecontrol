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

#include "actorprofile.h"
#include <RoomControlServer.h>
#include <factory.h>
#include <profile/serviceproviderprofile.h>

ActorProfile::ActorProfile(QObject* parent)
        : AbstractActor(parent) {}

void ActorProfile::execute()
{
    AbstractServiceProvider* p = RoomControlServer::getFactory()->get(m_id);
    ProfileCollection* pp = qobject_cast<ProfileCollection*>(p);
    if (!pp) return;
    if (m_action==StartProfile)
      pp->run();
    else if (m_action==CancelProfile)
      pp->cancel();
}

QString ActorProfile::profileid() const {
    return m_id;
}

void ActorProfile::setProfileid(const QString& value) {
    m_id = value;
}

int ActorProfile::action() const {
    return m_action;
}

void ActorProfile::setAction(int value) {
    m_action = value;
}