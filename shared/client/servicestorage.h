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


#pragma once
#include <QObject>
#include <QMap>

class AbstractStateTracker;
class AbstractServiceProvider;

class ServiceStorage : public QObject
{
    Q_OBJECT
public:
    static ServiceStorage* instance(bool first = false);
    static ServiceStorage* instance(ServiceStorage* instance);
    virtual ~ServiceStorage();
    // Return all services
    QMap<QString, AbstractServiceProvider*> services() const;
    AbstractServiceProvider* get(const QString& id);
    // For networkmanager: add new service
    void addService(AbstractServiceProvider* service);
    // For networkmanager: remove service
    void removeService(AbstractServiceProvider* service, bool emitRemoveSignal);
    // For networkmanager: update service
    void serviceUpdated(AbstractServiceProvider* service);
    // For networkmanager: update service
    void stateTrackerState(AbstractStateTracker* state);

    // Clear this service storage. Emits the servicesCleared signal.
    void clear();
    // For plugins: a service has changed. Emits serviceSync signal.
    void serviceHasChanged(AbstractServiceProvider* service);
    // For plugins: remove a service. Sets some properties and emits serviceSync signal.
    void deleteService(AbstractServiceProvider*);
private:
    static ServiceStorage* m_instance;
    ServiceStorage();
    // service providers and stateTracker
    QList<AbstractServiceProvider*> m_servicesList;
    QMap<QString, AbstractServiceProvider*> m_services;
Q_SIGNALS:
    // (networkmanager) a plugin changed/wants to remove a service,  propagate to server
    void serviceSync(AbstractServiceProvider* p);
    // (networkmanager) tell server to execute
    void serviceExecute(AbstractServiceProvider*);

    // the server changed a statetracker,  propagate to everone else
    void stateTrackerChanged(AbstractStateTracker*);
    // the server removed a service,  propagate to everone else
    void serviceRemoved(AbstractServiceProvider*);
    // the server changed a service,  propagate to everone else
    void serviceChanged(AbstractServiceProvider*);
    // disconnect from server: all services cleared
    void servicesCleared();
};
