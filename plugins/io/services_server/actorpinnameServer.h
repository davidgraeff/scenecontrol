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

#ifndef ActorPinNameServer_h
#define ActorPinNameServer_h
#include <executeservice.h>

class ActorPinName;
class myPluginExecute;
class ActorPinNameServer : public ExecuteService
{
  Q_OBJECT
public:
  ActorPinNameServer(ActorPinName* base, myPluginExecute* plugin, QObject* parent = 0);
  virtual bool checkcondition(){return true;}
  virtual void dataUpdate(){}
  virtual void execute();
private:
  myPluginExecute* m_plugin;
};

#endif // ActorPinNameServer_h
