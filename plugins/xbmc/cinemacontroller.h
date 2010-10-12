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

#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include "stateTracker/abstractstatetracker.h"

class CXBMCClient;
class QDBusPendingCallWatcher;
class CinemaStateTracker;
class CinemaController : public QObject
{
    Q_OBJECT
public:
    CinemaController();
    ~CinemaController();
    QList<AbstractStateTracker*> getStateTracker();

    /** 
      * if cinemaID is empty use all cinema players as targets
      */
    void setCommand(int cmd);
    void setPosition(int pos, bool relative=false);
    void setVolume(int vol, bool relative=false);
  private:
    CinemaStateTracker* m_statetracker;
    CXBMCClient* m_xbmcClient;
};

#endif // VIDEOCONTROLLER_H
