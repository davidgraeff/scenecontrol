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

#include "actorcurtain.h"

ActorCurtain::ActorCurtain(QObject* parent)
        : AbstractActor(parent)
{}

unsigned int ActorCurtain::value() const
{
    return m_value;
}

void ActorCurtain::setValue(unsigned int value)
{
    m_value = value;
}
void ActorCurtain::changed() {
    if (m_value==0)
        m_string = tr("Öffne Vorhang");
    else
        m_string = tr("Schließe Vorhang");
    AbstractServiceProvider::changed();
}
