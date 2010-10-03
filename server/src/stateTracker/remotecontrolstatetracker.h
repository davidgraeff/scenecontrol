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

#ifndef REMOTECONTROLSTATETRACKER_H
#define REMOTECONTROLSTATETRACKER_H
#include "abstractstatetracker.h"

class RemoteControlStateTracker : public AbstractStateTracker
{
  Q_OBJECT
  Q_PROPERTY(bool connected READ connected);
  Q_PROPERTY(int receivers READ receivers);
  Q_PROPERTY(QString mode READ mode);
  public:
    RemoteControlStateTracker(QObject* parent = 0);
    bool connected() const;
    int receivers() const;
    QString mode() const;
};

#endif // REMOTECONTROLSTATETRACKER_H
