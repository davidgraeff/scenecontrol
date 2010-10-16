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

#ifndef EXECUTEPLUGIN_H
#define EXECUTEPLUGIN_H
#include <QObject>
#include <QStringList>

class AbstractStateTracker;
class AbstractPlugin;
class ExecuteWithBase;
class ExecutePlugin : public QObject
{
	Q_OBJECT
public:
	virtual void refresh() = 0;
	virtual ExecuteWithBase* createExecuteService(const QString& id) = 0;
	virtual QList<AbstractStateTracker*> stateTracker() = 0;
	virtual AbstractPlugin* base() = 0;
Q_SIGNALS:
	void stateChanged(AbstractStateTracker*);
};
Q_DECLARE_INTERFACE(ExecutePlugin, "com.roomcontrol.ServerPlugin/1.0")
#endif // EXECUTEPLUGIN_H
