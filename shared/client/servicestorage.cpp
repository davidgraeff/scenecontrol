/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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


#include "servicestorage.h"
#include <shared/abstractserviceprovider.h>
#include <QVariant>
#include <shared/abstractstatetracker.h>

ServiceStorage* ServiceStorage::m_instance = 0;

ServiceStorage* ServiceStorage::instance(bool first) {
    if (!m_instance) {
        if (!first) {
            qFatal("ServiceStorage not existing!");
            return 0;
        }
        m_instance=new ServiceStorage();
    }
    return m_instance;
}

ServiceStorage* ServiceStorage::instance(ServiceStorage* instance) {
    m_instance=instance;
    return instance;
}

ServiceStorage::ServiceStorage()
{

}

ServiceStorage::~ServiceStorage()
{
    clear();
}

void ServiceStorage::clear() {
    qDeleteAll(m_services);
    m_services.clear();
    m_servicesList.clear();
    emit servicesCleared();
}

QMap< QString, AbstractServiceProvider* > ServiceStorage::services() const {
    return m_services;
}


AbstractServiceProvider* ServiceStorage::get(const QString& id) {
    return m_services.value ( id );
}

void ServiceStorage::networkRemove(AbstractServiceProvider* service, bool emitRemoveSignal) {
    if (!service) return;
    m_services.remove(service->id());
    m_servicesList.removeAll(service);
    if (emitRemoveSignal) emit serviceRemoved(service);
}

void ServiceStorage::networkUpdate(AbstractServiceProvider* service) {
    emit serviceChanged(service);
}

void ServiceStorage::networkAdd(AbstractServiceProvider* service) {
    m_services.insert ( service->id(), service );
    m_servicesList.append ( service );
    emit serviceChanged(service);
}

void ServiceStorage::deleteService(AbstractServiceProvider* service) {
    if (!service) return;
    service->setProperty("remove",true);
    emit serviceSync(service);
}

void ServiceStorage::networkUpdate(AbstractStateTracker* state) {
    emit stateTrackerChanged(state);
}

void ServiceStorage::serviceHasChanged(AbstractServiceProvider* service) {
    emit serviceSync(service);
}

void ServiceStorage::executeService(AbstractServiceProvider* service) {
    if (!service) return;
    service->setProperty("iexecute",true);
    emit serviceExecute(service);
}


