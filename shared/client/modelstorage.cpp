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


#include "modelstorage.h"
#include "clientplugin.h"
#include <shared/abstractserviceprovider.h>
#include <qalgorithms.h>
#include "servicestorage.h"
#include <shared/abstractstatetracker.h>
#include <QDebug>

ModelStorage* ModelStorage::m_instance = 0;

ModelStorage* ModelStorage::instance(bool first) {
    if (!m_instance) {
        if (!first) {
	  qFatal("ModelStorage not existing!");
	  return 0;
	}
        m_instance=new ModelStorage();
    }
    return m_instance;
}

ModelStorage* ModelStorage::instance(ModelStorage* instance) {
    m_instance=instance;
    return instance;
}

ModelStorage::ModelStorage()
{

}

ModelStorage::~ModelStorage()
{
    qDeleteAll(m_models);
    m_models.clear();
    m_modelsMap.clear();
}

ClientModel* ModelStorage::model(const QString& id) {
    return m_modelsMap.value(id);
}

void ModelStorage::registerClientModel(ClientModel* model, bool permanent) {
    ServiceStorage* servicestorage = ServiceStorage::instance();
    connect(servicestorage, SIGNAL(serviceChanged(AbstractServiceProvider*)), model, SLOT(serviceChanged(AbstractServiceProvider*)));
    connect(servicestorage, SIGNAL(serviceRemoved(AbstractServiceProvider*)), model, SLOT(serviceRemoved(AbstractServiceProvider*)));
    connect(servicestorage, SIGNAL(stateTrackerChanged(AbstractStateTracker*)), model, SLOT(stateTrackerChanged(AbstractStateTracker*)));
    connect(servicestorage, SIGNAL(servicesCleared()), model, SLOT(clear()));
    QMap<QString, AbstractServiceProvider*> services = servicestorage->services();
    foreach(AbstractServiceProvider* service, services) {
        model->serviceChanged(service);
    }
    if (permanent) {
        m_models.append(model);
        m_modelsMap.insert(model->id(), model);
    }
}

QMap< QString, ClientModel* > ModelStorage::models() const {
    return m_modelsMap;
}

