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

#include "eventcardreader.h"

EventCardReader::EventCardReader(QObject* parent)
        : AbstractServiceProvider(parent), m_state(0)
{

}

QString EventCardReader::atr()
{
    return m_atr;
}

void EventCardReader::setAtr(QString value)
{
    m_atr = value;
}

void EventCardReader::setState(int value)
{
    m_state = value;
}

int EventCardReader::state()
{
    return m_state;
}
