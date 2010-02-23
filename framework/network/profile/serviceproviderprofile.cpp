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
    m_events_model = new ServiceProviderModel(tr("Ereignisse"), this);
    m_conditions_model = new ServiceProviderModel(tr("Bedingungen"),this);
    m_events_and_conditions_model = new ServiceProviderModel(tr("Ereignisse/Bedingungen"),this);
    m_actors_model = new ServiceProviderModel(tr("Aktionen"),this);
}

void ProfileCollection::addChild(AbstractServiceProvider* provider)
{
    AbstractActor* a = qobject_cast<AbstractActor*>(provider);
    AbstractCondition* c = qobject_cast<AbstractCondition*>(provider);
    AbstractEvent* e = qobject_cast<AbstractEvent*>(provider);
    if (a) {
        m_actors_linked.insert(a->delay(), a);
	m_actors_model->addedProvider(provider);
    }
    if (c) {
        m_conditions_linked.insert(c);
        m_conditions_model->addedProvider(provider);
        m_events_and_conditions_model->addedProvider(provider);
    }
    if (e) {
        m_events_linked.insert(e);
        m_events_model->addedProvider(provider);
        m_events_and_conditions_model->addedProvider(provider);
    }
    emit childsChanged(this);
}

void ProfileCollection::removedChild(AbstractServiceProvider* provider)
{
    AbstractActor* a = qobject_cast<AbstractActor*>(provider);
    AbstractCondition* c = qobject_cast<AbstractCondition*>(provider);
    AbstractEvent* e = qobject_cast<AbstractEvent*>(provider);
    if (a) {
        m_actors_linked.remove(a->delay(), a);
	m_actors_model->removedProvider(provider);
    }
    if (c) {
        m_conditions_linked.remove(c);
        m_conditions_model->removedProvider(provider);
        m_events_and_conditions_model->removedProvider(provider);
    }
    if (e) {
        m_events_linked.remove(e);
        m_events_model->removedProvider(provider);
        m_events_and_conditions_model->removedProvider(provider);
    }
    emit childsChanged(this);
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
QString ProfileCollection::name() const {
    return m_name;
}
void ProfileCollection::setName(const QString& cmd) {
    m_name = cmd;
    m_string = cmd;
}
  