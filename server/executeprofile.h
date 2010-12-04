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

#ifndef EXECUTE_H
#define EXECUTE_H
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QMap>
#include <QSet>
#include "shared/server/executeWithBase.h"

class ExecuteService;
class ExecuteCollection : public ExecuteWithBase
{
    Q_OBJECT
public:
    ExecuteCollection(AbstractServiceProvider* p, QObject* parent = 0);
    virtual ~ExecuteCollection();
    /** Called if a child of this profile is going to be removed from disk */
    void removeChild ( ExecuteService* provider);
    /** Called by a child, that register itself as a child for this profile */
    void registerChild ( ExecuteService* provider);
    void stop();
    void run();
    QSet<ExecuteService*> m_childs_linked;
private Q_SLOTS:
    void eventTriggered();
    void executiontimeout();
private:
    QTimer m_executionTimer;
    int m_currentExecution;
    QMultiMap<int, ExecuteService*> m_actors_linked_map;
    QList<int> m_actors_delays;
Q_SIGNALS:
	void executeservice(ExecuteService*);
};

#endif // EXECUTESERVICEPROVIDER_H
