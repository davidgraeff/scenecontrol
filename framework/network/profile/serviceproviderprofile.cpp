/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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
#include "factory.h"
#include "RoomControlClient.h"
#include "actors/abstractactor.h"
#include "conditions/abstractcondition.h"
#include "events/abstractevent.h"
#include <models/serviceprovidermodel.h>

ProfileCollection::ProfileCollection(QObject* parent)
        : AbstractServiceProvider(parent), m_enabled(true)
{
    Factory* f = RoomControlClient::getFactory();
    connect(f,SIGNAL(addedProvider(AbstractServiceProvider*)),SLOT(addedProvider(AbstractServiceProvider*)));
    connect(f,SIGNAL(removedProvider(AbstractServiceProvider*)),SLOT(removedProvider(AbstractServiceProvider*)));
    m_events_model = new ServiceProviderModel(tr("Ereignisse"), this);
    m_conditions_model = new ServiceProviderModel(tr("Bedingungen"),this);
    m_events_and_conditions_model = new ServiceProviderModel(tr("Ereignisse/Bedingungen"),this);
    m_actors_model = new ServiceProviderModel(tr("Aktionen"),this);
}

void ProfileCollection::addedProvider(AbstractServiceProvider* provider)
{
    if (m_actors.contains(provider->id())) {
        AbstractActor* p = (AbstractActor*)provider;
        m_actors_linked.insert(p->delay(), p);
        m_actors_model->addedProvider(provider);
    } else if (m_conditions.contains(provider->id())) {
        m_conditions_linked.insert((AbstractCondition*)provider);
        m_conditions_model->addedProvider(provider);
	m_events_and_conditions_model->addedProvider(provider);
    } else if (m_events.contains(provider->id())) {
        AbstractEvent* p = (AbstractEvent*)provider;
        m_events_linked.insert(p);
        m_events_model->addedProvider(provider);
	m_events_and_conditions_model->addedProvider(provider);
    }
}

void ProfileCollection::removedProvider(AbstractServiceProvider* provider)
{
    if (m_actors.contains(provider->id())) {
        AbstractActor* p = (AbstractActor*)provider;
        m_actors_linked.remove(p->delay(),p);
        m_actors.removeAll(p->id());
        m_actors_model->removedProvider(provider);
    } else if (m_conditions.contains(provider->id())) {
        AbstractCondition* p = (AbstractCondition*)provider;
        m_conditions_linked.remove(p);
        m_conditions.removeAll(p->id());
        m_conditions_model->removedProvider(provider);
	m_events_and_conditions_model->removedProvider(provider);
    } else if (m_events.contains(provider->id())) {
        AbstractEvent* p = (AbstractEvent*)provider;
        m_events_linked.remove(p);
        m_events.removeAll(p->id());
        m_events_model->removedProvider(provider);
	m_events_and_conditions_model->removedProvider(provider);
    } else return;

    sync();
}

QString ProfileCollection::name() const {
    return m_name;
}
void ProfileCollection::setName(const QString& cmd) {
    m_name = cmd;
}
QStringList ProfileCollection::actors() const {
    return m_actors;
}
void ProfileCollection::setActors(const QStringList& cmd) {
    m_actors = cmd;
}
QStringList ProfileCollection::conditions() const {
    return m_conditions;
}
void ProfileCollection::setConditions(const QStringList& cmd) {
    m_conditions = cmd;
}
QStringList ProfileCollection::events() const {
    return m_events;
}
void ProfileCollection::setEvents(const QStringList& cmd) {
    m_events = cmd;
}
void ProfileCollection::changed() {
    m_string = m_name;
    if (!m_enabled) m_string.append(tr(" <Disabled>"));
    AbstractServiceProvider::changed();
}

ServiceProviderModel* ProfileCollection::actors_model() const {
    return m_actors_model;
}

ServiceProviderModel* ProfileCollection::conditions_model() const {
    return m_conditions_model;
}

ServiceProviderModel* ProfileCollection::events_model() const {
    return m_events_model;
}

ServiceProviderModel* ProfileCollection::events_and_conditions_model() const {
    return m_events_and_conditions_model;
}

void ProfileCollection::addChild(AbstractServiceProvider* provider)
{
    if (qobject_cast<AbstractActor*>(provider))
        m_actors.append(provider->id());
    else if (qobject_cast<AbstractCondition*>(provider))
        m_conditions.append(provider->id());
    else if (qobject_cast<AbstractEvent*>(provider))
        m_events.append(provider->id());
}
