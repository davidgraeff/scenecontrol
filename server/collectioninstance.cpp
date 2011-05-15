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

#include "collectioninstance.h"
#include <QDebug>
#include <QCoreApplication>
#include "servicecontroller.h"
#include "plugincontroller.h"

CollectionInstance::CollectionInstance ( ServiceStruct* service, ServiceController* sc) : m_collection(service), m_servicecontroller ( sc ) {
    m_executionTimer.setSingleShot ( true );
    connect ( &m_executionTimer, SIGNAL ( timeout() ),SLOT ( executiontimeout() ) );
}

CollectionInstance::~CollectionInstance() {
	while (m_serviceids.size()) {
        removeService(ServiceType::uniqueID(m_serviceids.begin().value()->data));
	}
}

void CollectionInstance::change(const QVariantMap& data, const QVariantMap& olddata) {
    QVariantList newlist = data[ QLatin1String("services") ].toList();
    QSet<QString> newservices;
    foreach ( QVariant v, newlist ) {
        newservices.insert ( v.toString() );
    }
    QVariantList oldlist = olddata[ QLatin1String("services") ].toList();
    QSet<QString> oldservices;
    foreach ( QVariant v, oldlist ) {
        oldservices.insert ( v.toString() );
    }

    // remove not used services
    oldservices.subtract(newservices);
    foreach ( QString uid, oldservices ) {
        removeService(uid);
    }

    // add new services
    newservices.subtract(QSet<QString>::fromList(m_serviceids.keys()));
    foreach ( QString uid, newservices ) {
        ServiceStruct* service = m_servicecontroller->service(uid);
        if (!service) {
            qWarning()<<"Collection contains not existing service"<< m_collection->data<<uid;
            continue;
        }
        changeService(service, service->data, QVariantMap());
    }

    m_enabled = BOOLDATA ( "enabled" );
}

void CollectionInstance::changeService ( ServiceStruct* service, const QVariantMap& data, const QVariantMap& olddata ) {
    ServiceStruct* oldservice = m_serviceids.value(ServiceType::uniqueID(data));
    if (!oldservice || oldservice != service) {
        m_serviceids[ServiceType::uniqueID(data)] = service;
    }

    if (!ServiceType::isEvent(data)) return;

    if (olddata.size() && olddata != data) {
        qDebug() << "unregister event";
        service->plugin->unregister_event(service->data, ServiceType::uniqueID(m_collection->data));
    }

    service->plugin->register_event(service->data, ServiceType::uniqueID(m_collection->data));

}

void CollectionInstance::removeService(const QString& uid)
{
    ServiceStruct* service = m_serviceids.take(uid);
    if (!service) return;
    service->inCollections.remove(this);

    if (!ServiceType::isEvent(service->data)) return;
    service->plugin->unregister_event(service->data, ServiceType::uniqueID(m_collection->data));
}

void CollectionInstance::start()
{
    if (!m_enabled) return;

    // conditions
    QMap<QString, bool> conditionGroups;

    // build conditionGroups (within a group conditions are or'd)
    {
        QMap<QString, ServiceStruct*>::iterator i =  m_serviceids.begin();
        for (;i!=m_serviceids.end();i++) {
            if (!ServiceType::isCondition(i.value()->data))
                continue;
            const QString conditionGroup = ServiceType::conditionGroup( i.value()->data );
            bool c =i.value()->plugin->condition ( i.value()->data, QString() );
			if (ServiceType::isNegatedCondition(i.value()->data)) c = !c;
            if ( c ) {
                conditionGroups[conditionGroup] = true;
            } else {
                conditionGroups[conditionGroup] |= false;
            }
        }
    }

    // check term (all conditionGroups are and'd)
    {
        QMap<QString, bool>::iterator i =  conditionGroups.begin();
        for (;i!=conditionGroups.end();i++) {
            if (!i.value())
                return;
        }
    }

    execute();
}

void CollectionInstance::execute() {
    m_executionTimer.stop();

    /* generate execution order */
    m_executionids.clear();
    foreach ( ServiceStruct* service, m_serviceids ) {
        if (!ServiceType::isAction(service->data))
            continue;
        const int delay = service->data.value ( QLatin1String ( "__delay" ) ).toInt();
        m_executionids.insertMulti ( delay, service );
    }

    QMap< int, ServiceStruct* >::iterator it = m_executionids.lowerBound ( 0 );
    if ( it == m_executionids.end() ) return;
    m_currenttime = it.key();
    m_executionTimer.start ( 1000*m_currenttime );
}

void CollectionInstance::executiontimeout() {
    const QList<ServiceStruct*> executeservices = m_executionids.values ( m_currenttime );
    foreach ( ServiceStruct* service, executeservices ) {
		service->plugin->execute(service->data, QString());
    }

    ++m_currenttime;
    QMap< int, ServiceStruct* >::iterator it = m_executionids.lowerBound ( m_currenttime );
    if ( it == m_executionids.end() ) return;
    m_currenttime = it.key();
    m_executionTimer.start ( 1000*m_currenttime );
}

void CollectionInstance::stop() {
    m_executionTimer.stop();
}

void CollectionInstance::clone() {
    QVariantMap newcollection = m_servicecontroller->cloneService(m_collection);
    if (newcollection.isEmpty()) return;
	
    QString newcollectionuid = ServiceType::uniqueID(newcollection);
    newcollection[QLatin1String("name")] = QString(newcollection[QLatin1String("name")].toString() +  QLatin1String(" (copy)"));
    newcollection[QLatin1String("enabled")] = true;
    newcollection[QLatin1String("services")] = QVariantList();
    m_servicecontroller->changeService(newcollection, QString());

    foreach(ServiceStruct* service, m_serviceids) {
        QVariantMap newservice = m_servicecontroller->cloneService(service);
        ServiceType::setToCollection(newservice, newcollectionuid);
        m_servicecontroller->changeService(newservice, QString());
    }
}
