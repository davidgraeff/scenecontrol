/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef myPluginClient_H
#define myPluginClient_H
#include <QObject>
#include <QStringList>
#include "shared/client/clientplugin.h"
#include <shared/abstractplugin.h>

class EventStateTracker;
class ModeStateTracker;
class ExecuteWithBase;
class SystemStateTracker;
class BackupStateTracker;
class ExecuteService;
class ServiceController;
class EventVolumeStateTracker;
class myPluginClient : public ClientPlugin
{
    Q_OBJECT
public:
    myPluginClient(QObject* parent = 0);
    virtual ~myPluginClient();
    virtual AbstractPlugin* base() {
        return m_base;
    }

private:
    AbstractPlugin* m_base;
};

#endif // myPluginClient_H
