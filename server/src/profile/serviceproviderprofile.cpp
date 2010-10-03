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

#include "serviceproviderprofile.h"
#include <QDebug>
#include "factory.h"
#include "RoomControlServer.h"
#include "actors/abstractactor.h"
#include "conditions/abstractcondition.h"
#include "events/abstractevent.h"
#include <actors/actorcurtain.h>
#include <actors/actorpin.h>
#include <networkcontroller.h>

ProfileCollection::ProfileCollection(QObject* parent)
        : AbstractServiceProvider(parent), m_enabled(true)
{
    m_currentExecution = 0;
    m_executionTimer.setSingleShot(true);
    connect(&m_executionTimer, SIGNAL(timeout()),SLOT(timeout()));
}

void ProfileCollection::registerChild(AbstractServiceProvider* provider)
{
    AbstractActor* a = qobject_cast<AbstractActor*>(provider);
    AbstractCondition* c = qobject_cast<AbstractCondition*>(provider);
    AbstractEvent* e = qobject_cast<AbstractEvent*>(provider);
    if (a) m_actors_linked.insert(a);
    else if (c) m_conditions_linked.insert(c);
    else if (e) {
        m_events_linked.insert(e);
        e->disconnect();
        connect(e, SIGNAL(eventTriggered()),SLOT(eventTriggered()));
    } else
        qWarning()<<__FUNCTION__<<"not known child type";
}

void ProfileCollection::removeFromDisk() {
    // first remove this profile
    RoomControlServer::getFactory()->objectRemoveFromDisk(this);
    // and then all its children; In this order
    // to prevend endless childRemoved calls
    foreach (AbstractActor* a, m_actors_linked) a->removeFromDisk();
    foreach (AbstractCondition* a, m_conditions_linked) a->removeFromDisk();
    foreach (AbstractEvent* a, m_events_linked) a->removeFromDisk();
}

void ProfileCollection::childRemoved(AbstractServiceProvider* provider)
{
    AbstractActor* a = qobject_cast<AbstractActor*>(provider);
    AbstractCondition* c = qobject_cast<AbstractCondition*>(provider);
    AbstractEvent* e = qobject_cast<AbstractEvent*>(provider);
    if (a) m_actors_linked.remove(a);
    if (c) m_conditions_linked.remove(c);
    if (e) {
		m_events_linked.remove(e);
		e->disconnect( SIGNAL(eventTriggered()), this);
	}
}

void ProfileCollection::run()
{
    if (m_actors_linked.isEmpty()) return;
    m_currentExecution = 0;
	m_actors_linked_map.clear();
	foreach (AbstractActor* a, m_actors_linked)
		m_actors_linked_map.insert(a->delay(), a);
    m_actors_delays = m_actors_linked_map.uniqueKeys();
    m_executionTimer.start(1000*m_actors_delays[m_currentExecution]);
}

void ProfileCollection::eventTriggered()
{
    // only run if enabled
    if (!m_enabled) return;

    // check conditions
    foreach (AbstractCondition* p, m_conditions_linked)
    {
        if (!p->ok())
        {
            return;
        }
    }

    run();
}

void ProfileCollection::timeout()
{
    const int currentTimePoint = m_actors_delays[m_currentExecution];
    QList<AbstractActor*> actors = m_actors_linked_map.values(currentTimePoint);
    foreach (AbstractActor* p, actors)
    {
        p->execute();
    }
    if (++m_currentExecution >= m_actors_delays.size())
        return;

    const int nextTimePoint = m_actors_delays[m_currentExecution];
    m_executionTimer.start(1000*(nextTimePoint-currentTimePoint));
}

bool ProfileCollection::isRunning()
{
    return m_executionTimer.isActive();
}

void ProfileCollection::cancel()
{
    m_executionTimer.stop();
}
