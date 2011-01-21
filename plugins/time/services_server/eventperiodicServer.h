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

#ifndef EventPeriodicServer_h
#define EventPeriodicServer_h
#include <QVector>
#include <QVariantMap>
#include <QTimer>
#include "shared/server/executeservice.h"

class myPluginExecute;
class EventPeriodic;
class EventPeriodicServer : public ExecuteService
{
    Q_OBJECT
public:
    EventPeriodicServer(EventPeriodic* base, myPluginExecute* plugin, QObject* parent = 0);
    virtual bool checkcondition();
    virtual void dataUpdate();
    virtual void execute();
    virtual void nameUpdate();
  private:
    QTimer m_timer;
    bool m_aftertrigger;
  private Q_SLOTS:
    void timeout(bool aftersync=false);
};

#endif // EventPeriodicServer_h
