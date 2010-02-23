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

#include "conditioneventstate.h"
#include <RoomControlClient.h>

ConditionEventState::ConditionEventState(QObject* parent)
: AbstractCondition(parent)
{  
}

int ConditionEventState::value() const
{
    return m_value;
}

void ConditionEventState::setValue(int value)
{
    m_value = (int)value;
}
void ConditionEventState::changed() {
    if (m_value==RoomControlClient::PlayState)
        m_string = tr("Wenn Eventsound abgespielt wird");
    else if (m_value==RoomControlClient::StopState)
        m_string = tr("Wenn Eventsound gestoppt");
    AbstractServiceProvider::changed();
}
