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

#include "conditionpin.h"

ConditionPin::ConditionPin(QObject* parent)
: AbstractCondition(parent)
{  
}

unsigned int ConditionPin::pin() const
{
    return m_pin;
}

void ConditionPin::setPin(unsigned int value)
{
    m_pin = value;
}

bool ConditionPin::value() const
{
    return m_value;
}

void ConditionPin::setValue(bool value)
{
    m_value = value;
}
void ConditionPin::changed() {
    if (m_value)
        m_string = tr("Wenn Pin %1 ein ist.").arg(m_pin);
    else
        m_string = tr("Wenn Pin %1 aus ist.").arg(m_pin);
    AbstractServiceProvider::changed();
}
