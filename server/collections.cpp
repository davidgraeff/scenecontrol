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

#include "executeprofile.h"
#include "shared/server/executeservice.h"
#include <shared/categorize/profile.h>
#include <QDebug>

ExecuteCollection::ExecuteCollection(AbstractServiceProvider* p, QObject* parent) :ExecuteWithBase(p,parent) {
    m_currentExecution = 0;
    m_executionTimer.setSingleShot ( true );
    connect ( &m_executionTimer, SIGNAL ( timeout() ),SLOT ( executiontimeout() ) );
}

ExecuteCollection::~ExecuteCollection() {
}

void ExecuteCollection::registerChild(ExecuteService* service) {
    m_childs_linked.insert(service);
    if (service->base()->service() == AbstractServiceProvider::EventService)
        connect(service,SIGNAL(trigger()),SLOT(eventTriggered()));
}

void ExecuteCollection::removeChild(ExecuteService* service) {
    if (!m_childs_linked.contains(service)) return;
    m_childs_linked.remove(service);
    if (service->base()->service() == AbstractServiceProvider::EventService)
        disconnect(service,SIGNAL(trigger()),this, SLOT(eventTriggered()));
}

void ExecuteCollection::stop() {
    m_executionTimer.stop();
}
void ExecuteCollection::executiontimeout()
{
    const int currentTimePoint = m_actors_delays[m_currentExecution];
    QList<ExecuteService*> actors = m_actors_linked_map.values ( currentTimePoint );
    foreach ( ExecuteService* p, actors )
    {
        emit executeservice(p);
    }
    if ( ++m_currentExecution >= m_actors_delays.size() )
        return;

    const int nextTimePoint = m_actors_delays[m_currentExecution];
    m_executionTimer.start ( 1000* ( nextTimePoint-currentTimePoint ) );
}
void ExecuteCollection::run(bool ignoreConditions) {
	if (!ignoreConditions && !service<Collection>()->enabled() ) return;

    // check conditions
    if (!ignoreConditions) {
        foreach ( ExecuteService* service, m_childs_linked )
        {
            if ((service->base()->service() == AbstractServiceProvider::ConditionService ) && !service->checkcondition() )
            {
                return;
            }
        }
    }

    m_executionTimer.stop();
    m_currentExecution = 0;
    m_actors_linked_map.clear();
    foreach ( ExecuteService* service, m_childs_linked )
    {
        if ( (service->base()->service() == AbstractServiceProvider::ActionService ) )
            m_actors_linked_map.insert ( service->base()->delay(), service );
    }
    if ( m_actors_linked_map.isEmpty() ) return;
    m_actors_delays = m_actors_linked_map.uniqueKeys();
    m_executionTimer.start ( 1000*m_actors_delays[m_currentExecution] );
}
void ExecuteCollection::eventTriggered() {
    run();
}
