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

#include "actorcinema.h"
#include <QMetaProperty>
#include <QDebug>
#include <RoomControlClient.h>

ActorCinema::ActorCinema(QObject* parent)
        : AbstractActor(parent)
{
}

int ActorCinema::cmd() const {
    return m_cmd;
}
void ActorCinema::setCmd(int value) {
    m_cmd = value;
}

void ActorCinema::changed() {
    QMetaEnum e = RoomControlClient::staticMetaObject.enumerator(RoomControlClient::staticMetaObject.indexOfEnumerator("EnumMediaCmd"));
    QString name = QString::number(m_cmd);
    if (e.isValid())
        name = e.valueToKey(m_cmd);

    m_string = tr("Cinema Kommando %1").arg(name);
    AbstractServiceProvider::changed();
}
