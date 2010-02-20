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

#include "remotecontrolstatetracker.h"

RemoteControlStateTracker::RemoteControlStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{}
bool RemoteControlStateTracker::connected() const {
    return m_connected;
}
int RemoteControlStateTracker::receivers() const {
    return m_receivers;
}
void RemoteControlStateTracker::setConnected(bool c) {
    m_connected = c;
}
void RemoteControlStateTracker::setReceivers(int r) {
    m_receivers = r;
}
const QString& RemoteControlStateTracker::mode() const {
    return m_mode;
}
void RemoteControlStateTracker::setMode(const QString& m) {
    m_mode = m;
}

