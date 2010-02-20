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

#include "actorled.h"

ActorLed::ActorLed(QObject* parent)
        : AbstractActor(parent), m_fadetype(1), m_assignment(ValueAbsolute)
{}

unsigned int ActorLed::channel() const
{
    return m_channel;
}

void ActorLed::setChannel(unsigned int value)
{
    m_channel = value;
}

unsigned int ActorLed::fadetype() const
{
    return m_fadetype;
}

void ActorLed::setFadetype(unsigned int value)
{
    m_fadetype = value;
}

int ActorLed::value() const
{
    return m_value;
}

void ActorLed::setValue(int value)
{
    m_value = value;
}

int ActorLed::assignment() const
{
    return m_assignment;
}

void ActorLed::setAssignment(int value)
{
    m_assignment = value;
}
void ActorLed::changed() {
    if (m_assignment==ValueAbsolute)
        m_string = tr("Led %1 auf %2").arg(m_channel).arg(m_value);
    else if (m_assignment==ValueRelative)
        m_string = tr("Led %1 verändern um %2").arg(m_channel).arg(m_value);
    else if (m_assignment==ValueMultiplikator)
        m_string = tr("Led %1 verändern um Faktor %2").arg(m_channel).arg(m_value);
    else if (m_assignment==ValueInverse)
        m_string = tr("Led %1 Wert invertieren").arg(m_channel);
    AbstractServiceProvider::changed();
}
