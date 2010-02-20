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

#include "conditionmusicstate.h"

ConditionMusicState::ConditionMusicState(QObject* parent)
: AbstractCondition(parent)
{  
}

int ConditionMusicState::value() const
{
    return m_value;
}

void ConditionMusicState::setValue(int value)
{
    m_value = (int)value;
}
void ConditionMusicState::changed() {
    if (m_value==PlayState)
        m_string = tr("Wenn Musik abgespielt wird");
    else if (m_value==StopState)
        m_string = tr("Wenn Musik gestoppt");
    else if (m_value==PauseState)
        m_string = tr("Wenn Musik pausiert");
    AbstractServiceProvider::changed();
}
