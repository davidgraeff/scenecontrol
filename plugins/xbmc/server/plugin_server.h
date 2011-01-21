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

#ifndef myPLUGINSERVER_H
#define myPLUGINSERVER_H
#include <QObject>
#include <QStringList>
#include "shared/server/executeplugin.h"

class CinemaPositionStateTracker;
class CinemaStateTracker;
class CinemaVolumeStateTracker;
class CXBMCClient;
class myPluginExecute : public ExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES(ExecutePlugin)
public:
    myPluginExecute();
    virtual ~myPluginExecute();
    virtual void refresh() ;
	virtual void clear(){}
    virtual ExecuteWithBase* createExecuteService(const QString& id);
    virtual QList<AbstractStateTracker*> stateTracker();
    virtual AbstractPlugin* base() {
        return m_base;
    }
    /**
      * if cinemaID is empty use all cinema players as targets
      */
    void setCommand(int cmd);
    void setPosition(int pos, bool relative=false);
    void setVolume(int vol, bool relative=false);
private:
    AbstractPlugin* m_base;
    CinemaStateTracker* m_CinemaStateTracker;
    CinemaPositionStateTracker* m_CinemaPositionStateTracker;
    CinemaVolumeStateTracker* m_CinemaVolumeStateTracker;
    CXBMCClient* m_xbmcClient;
};

#endif // myPLUGINSERVER_H
