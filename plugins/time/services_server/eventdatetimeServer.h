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

#ifndef EventDateTimeServer_h
#define EventDateTimeServer_h
#include <QTimer>
#include <executeservice.h>

class myPluginExecute;
class EventDateTime;
class EventDateTimeServer : public ExecuteService
{
    Q_OBJECT
public:
    EventDateTimeServer(EventDateTime* base, myPluginExecute* plugin,QObject* parent = 0);
    virtual bool checkcondition();
    virtual void dataUpdate();
    virtual void execute();
  private:
    QTimer m_timer;
  private Q_SLOTS:
    void timeout();
};

#endif // EventDateTimeServer_h
