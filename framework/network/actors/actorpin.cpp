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

#include "actorpin.h"
#include <RoomControlClient.h>

ActorPin::ActorPin(QObject* parent)
        : AbstractActor(parent)
{}

unsigned int ActorPin::pin() const
{
    return m_pin;
}

void ActorPin::setPin(unsigned int value)
{
    m_pin = value;
}

int ActorPin::value() const
{
    return m_value;
}

void ActorPin::setValue(int value)
{
    m_value = value;
}
void ActorPin::changed() {
    PinsModel* m = RoomControlClient::getPinsModel();
    QString pinname;
    if (m)
        pinname = m->getName(m_pin);
    if (pinname.isEmpty())
        pinname = tr("Pin %1").arg(m_pin);

    if (m_value==PinOff)
        m_string = tr("%1 auf aus").arg(pinname);
    else if (m_value==PinOn)
        m_string = tr("%1 auf an").arg(pinname);
    else if (m_value==PinToggle)
        m_string = tr("%1 wechseln").arg(pinname);
    AbstractServiceProvider::changed();
}
