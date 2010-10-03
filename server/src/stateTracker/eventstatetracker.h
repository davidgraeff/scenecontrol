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

#ifndef EventStateTracker_h
#define EventStateTracker_h
#include "abstractstatetracker.h"
#include "media/mediacmds.h"

class EventStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename);
    Q_PROPERTY(QString title READ title);
    Q_PROPERTY(int state READ state);
public:
    EventStateTracker(QObject* parent = 0);
    QString title() const;
    QString filename() const;
    int state();
};

#endif // EventStateTracker_h
