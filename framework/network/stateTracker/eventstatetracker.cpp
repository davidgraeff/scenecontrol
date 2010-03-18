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

#include "eventstatetracker.h"

EventStateTracker::EventStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{
}
void EventStateTracker::setState(int s) {
    m_state = s;
}
const QString& EventStateTracker::title() const {
    return m_title;
}
void EventStateTracker::setTitle(const QString& t) {
    m_title = t;
}
const QString& EventStateTracker::filename() const {
    return m_filename;
}
void EventStateTracker::setFilename(const QString& f) {
    m_filename = f;
}
int EventStateTracker::state() const {
    return m_state;
}

