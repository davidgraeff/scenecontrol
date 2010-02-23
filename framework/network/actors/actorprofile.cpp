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

#include "actorprofile.h"
#include <RoomControlClient.h>

ActorProfile::ActorProfile(QObject* parent)
        : AbstractActor(parent) {}

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

void ActorProfile::changed() {
    QString profilname = m_id;
    AbstractServiceProvider* p = RoomControlClient::getFactory()->get(m_id);
    if (p) profilname = p->toString();

    if (m_action==0)
        m_string = tr("Starte Profil %1").arg(profilname);
    else
        m_string = tr("Beende Profil %1").arg(profilname);
    AbstractActor::changed();
}
