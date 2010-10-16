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

#include "eventremotekey.h"

EventRemoteKey::EventRemoteKey(QObject* parent)
        : AbstractServiceProvider(parent), m_pressed(false), m_repeat(0), m_repeatinit(0), m_channel(-1)
{

}

bool EventRemoteKey::pressed()
{
    return m_pressed;
}

void EventRemoteKey::setPressed(bool value)
{
    m_pressed = value;
}

int EventRemoteKey::channel()
{
    return m_channel;
}

void EventRemoteKey::setChannel(int value)
{
    m_channel = value;
}

QString EventRemoteKey::key()
{
    return m_key;
}

void EventRemoteKey::setKey(QString value)
{
    m_key = value;
}

int EventRemoteKey::repeat() const {
    return m_repeat;
}
void EventRemoteKey::setRepeat(int r) {
    m_repeat = r;
}
