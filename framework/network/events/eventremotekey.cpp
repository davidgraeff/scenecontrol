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

#include "eventremotekey.h"

EventRemoteKey::EventRemoteKey(QObject* parent)
        : AbstractEvent(parent)
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
void EventRemoteKey::changed() {
    m_string = tr("Remote Taste: %1").arg(m_key);
    AbstractServiceProvider::changed();
}
