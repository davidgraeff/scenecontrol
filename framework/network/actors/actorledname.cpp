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

#include "actorledname.h"

ActorLedName::ActorLedName(QObject* parent)
        : AbstractActor(parent)
{}

unsigned int ActorLedName::channel() const
{
    return m_channel;
}

void ActorLedName::setChannel(unsigned int value)
{
    m_channel = value;
}

QString ActorLedName::ledname() const
{
    return m_ledname;
}

void ActorLedName::setLedname(const QString& ledname)
{
    m_ledname = ledname;
}
void ActorLedName::changed() {
    m_string = tr("Led %1 Namen auf %2").arg(m_channel).arg(m_ledname);
    AbstractServiceProvider::changed();
}
