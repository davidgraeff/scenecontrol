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

#ifndef ActorCinemaVolumeServer_h
#define ActorCinemaVolumeServer_h
#include "shared/server/executeservice.h"

class ActorCinemaVolume;
class myPluginExecute;
class ActorCinemaVolumeServer : public ExecuteService
{
  Q_OBJECT
public:
  ActorCinemaVolumeServer(ActorCinemaVolume* base, myPluginExecute* plugin, QObject* parent = 0);
  virtual bool checkcondition(){return true;}
  virtual void dataUpdate(){}
  virtual void execute();
private:
  myPluginExecute* m_plugin;
};

#endif // ActorCinemaVolumeServer_h
