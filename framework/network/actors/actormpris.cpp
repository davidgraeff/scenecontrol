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

#include "actormpris.h"
#include "media/mediacmds.h"

ActorMpris::ActorMpris(QObject* parent)
        : AbstractActor(parent)
{
}

int ActorMpris::cmd() const {
    return m_cmd;
}
void ActorMpris::setCmd(int value) {
    m_cmd = value;
}
QString ActorMpris::mprisid() const {
    return m_mprisid;
}
void ActorMpris::setMprisid(const QString& value) {
    m_mprisid = value;
}
void ActorMpris::changed() {
    m_string = tr("Mpris Kommando").arg(m_cmd);
    AbstractServiceProvider::changed();
}
