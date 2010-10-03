/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

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

#include "abstractactor.h"

AbstractActor::AbstractActor(QObject* parent)
        : AbstractServiceProvider(parent), m_iExecute(false), m_delay(0)
{}
int AbstractActor::delay() const {
    return m_delay;
}
void AbstractActor::setDelay(int cmd) {
    m_delay = cmd;
}
bool AbstractActor::iExecute() const {
    return m_iExecute;
}
void AbstractActor::setiExecute(bool cmd) {
    m_iExecute = cmd;
}
