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

// 	const QString conditions = DATA("conditions");
// 	if (conditions.size()) {
// 		// convert condition string to parsed binary decision diagram
// 		boolstuff::BoolExprParser parser;
// 		try {
// 			instance->conditionids = parser.parse(conditions.toStdString());
// 		} catch (boolstuff::BoolExprParser::Error e) {
// 			qWarning()<<"Collections: Parsing of conditions expression failed! UID: " << ServiceType::uniqueID(data);
// 		}
// 	}

//             std::set<std::string> positives, negatives;
//             instance->conditionids->getTreeVariables(positives, negatives);
//             bool ok = true;
//             foreach(std::string uid, positives) {
//                 ServiceController::ServiceStruct* s =  m_servicecontroller->service(QString::fromStdString(uid));
//                 if (!s) continue;
//                 if (!s->plugin->condition(s->data, QString())) {
//                     ok = false;
//                     break;
//                 }
//             }
//
//             if (!ok) continue;
//             foreach(std::string uid, negatives) {
//                 ServiceController::ServiceStruct* s =  m_servicecontroller->service(QString::fromStdString(uid));
//                 if (!s) continue;
//                 if (s->plugin->condition(s->data, QString())) {
//                     ok = false;
//                     break;
//                 }
//             }

#include "collections.h"
#include <QDebug>
#include <QCoreApplication>
#include "servicecontroller.h"
#include "boolstuff/BoolExprParser.h"
#include "plugincontroller.h"

Collections::Collections() : m_servicecontroller ( 0 ) {
}

Collections::~Collections() {
    qDeleteAll ( m_collections );
    m_collections.clear();
}

QList< QVariantMap > Collections::properties ( const QString& sessionid ) {
    Q_UNUSED ( sessionid );
    QList<QVariantMap> l;
    return l;
}

bool Collections::condition ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( data );
    Q_UNUSED ( sessionid );
    return false;
}

void Collections::register_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED ( data );
    Q_UNUSED ( collectionuid );
}

void Collections::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED ( data );
    Q_UNUSED ( collectionuid );
}

void Collections::execute ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isId ( data,"executecollection" ) ) {
        CollectionInstance* instance = m_collections.value ( DATA ( "collectionid" ) );
        if ( instance && instance->enabled ) instance->startExecution();
    } else if ( ServiceID::isId ( data,"stopcollection" ) ) {
        CollectionInstance* instance = m_collections.value ( DATA ( "collectionid" ) );
        if ( instance ) instance->stop();
    } else if ( ServiceID::isId ( data,"clone" ) ) {
        CollectionInstance* instance = m_collections.value ( DATA ( "collectionid" ) );
        if ( instance ) instance->clone();
    }
}

void Collections::initialize() {
}

void Collections::clear() {
}

void Collections::addCollection ( const QVariantMap& data ) {
    if ( m_servicecontroller->removedNotExistingServicesFromCollection ( data, true ) ) {
        // addCollection will be called again with new data, so exit here
        return;
    }

    CollectionInstance* instance = new CollectionInstance ( m_servicecontroller );
    instance->setData(data);
    m_collections.insert ( ServiceType::uniqueID ( data ), instance );

    connect ( instance,SIGNAL ( executeService ( QString, QString ) ),SIGNAL ( instanceExecute ( QString, QString ) ) );
}

void Collections::dataSync ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED ( sessionid );
    if ( ServiceType::isRemoveCmd ( data ) ) {
        delete m_collections.take ( ServiceType::uniqueID ( data ) );
        return;
    } else if ( ServiceType::isEvent ( data ) ) {
        const QString eventuid = ServiceType::uniqueID(data);
        QMap<QString, CollectionInstance*>::iterator i = m_collections.begin();
        for (;i!=m_collections.end();++i) {
            QSet<QString>::iterator eventit = i.value()->eventids.find(eventuid);
            if (i.value()->eventids.end() != eventit) {
                i.value()->reregisterEvents(eventuid);
             }
         }
         return;
    } else if ( !ServiceType::isCollection ( data ) )
        return;

    CollectionInstance*instance = m_collections.value ( ServiceType::uniqueID ( data ) );
    if (instance) {
        instance->setData(data);
    } else {
        addCollection ( data );
    }
}

void Collections::dataReady() {
    // clear
    qDeleteAll ( m_collections );
    m_collections.clear();

    // get collections
    const QMap< QString, ServiceController::ServiceStruct* > services = m_servicecontroller->valid_services();
    foreach ( ServiceController::ServiceStruct* service, services ) {
        const QVariantMap data = service->data;
        if ( ServiceType::isCollection ( data ) ) {
            addCollection ( data );
        }
    }
}

void Collections::eventTriggered ( const QString& uid, const QString& destination_collectionuid ) {
    CollectionInstance* instance = m_collections.value ( destination_collectionuid );
    if ( !instance || !instance->enabled || !instance->eventids.contains ( uid ) ) return;
    // conditions
    bool ok = true;
    foreach ( QString uid, instance->conditionids ) {
        ServiceController::ServiceStruct* s =  m_servicecontroller->service ( uid );
        if ( !s ) continue;
        if ( !s->plugin->condition ( s->data, QString() ) ) {
            ok = false;
            break;
        }
    }

    if ( !ok ) return;

    // execute
    instance->startExecution();
}

void Collections::convertVariantToStringSet ( const QVariantList& source, QSet<QString>& destination ) {
    foreach ( QVariant v, source ) {
        destination.insert ( v.toString() );
    }
}

void Collections::convertVariantToIntStringMap ( const QVariantMap& source, QMap< int, QString >& destination ) {
    QVariantMap::const_iterator i = source.constBegin();
    for ( ;i!=source.constEnd();++i )
        destination.insertMulti ( i.key().toInt(), i.value().toString() );
}

CollectionInstance::CollectionInstance ( ServiceController* sc) : conditionlinks ( 0 ), m_servicecontroller ( sc ) {
    m_executionTimer.setSingleShot ( true );
    connect ( &m_executionTimer, SIGNAL ( timeout() ),SLOT ( executiontimeout() ) );
}

CollectionInstance::~CollectionInstance() {
    QMap<QString,QVariantMap>::iterator i = m_eventdataCache.begin();
    for (;i!=m_eventdataCache.end();++i) {
        unregisterEvent(i.value());
    }
    delete conditionlinks;
}

void CollectionInstance::setData(const QVariantMap& data) {
    Collections::convertVariantToStringSet ( LIST ( "actions" ), actionids );
    Collections::convertVariantToStringSet ( LIST ( "conditions" ), conditionids );
    QSet<QString> oldevents = eventids;
    Collections::convertVariantToStringSet ( LIST ( "events" ), eventids );
    enabled = BOOLDATA ( "enabled" );
    collectionuid = ServiceType::uniqueID ( data );

    reregisterEvents();
}

void CollectionInstance::startExecution() {
    m_executionTimer.stop();

    /* generate execution order */
    executionids.clear();
    foreach ( QString serviceuid, actionids ) {
        ServiceController::ServiceStruct* service = m_servicecontroller->service ( serviceuid );
        const int delay = service->data.value ( QLatin1String ( "__delay" ) ).toInt();
        const QString uid = ServiceType::uniqueID ( service->data );
        executionids.insertMulti ( delay, uid );
    }

    QMap< int, QString >::iterator it = executionids.lowerBound ( 0 );
    if ( it == executionids.end() ) return;
    m_currenttime = it.key();
    m_executionTimer.start ( 1000*m_currenttime );
}

void CollectionInstance::executiontimeout() {
    const QList<QString> uids = executionids.values ( m_currenttime );
    foreach ( QString uid, uids ) {
        emit executeService ( uid, QString() );
    }

    ++m_currenttime;
    QMap< int, QString >::iterator it = executionids.lowerBound ( m_currenttime );
    if ( it == executionids.end() ) return;
    m_currenttime = it.key();
    m_executionTimer.start ( 1000*m_currenttime );
}

void CollectionInstance::stop() {
    m_executionTimer.stop();
}

void CollectionInstance::registerEvent(const QVariantMap& event) {
    AbstractPlugin_services* plugin = dynamic_cast<AbstractPlugin_services*>(m_servicecontroller->getPluginController()->getPlugin(ServiceID::pluginid(event)));
    if (!plugin) return;
    plugin->register_event(event, collectionuid);
}

void  CollectionInstance::unregisterEvent(const QVariantMap& event) {
    AbstractPlugin_services* plugin = dynamic_cast<AbstractPlugin_services*>(m_servicecontroller->getPluginController()->getPlugin(ServiceID::pluginid(event)));
    if (!plugin) return;
    plugin->unregister_event(event, collectionuid);
}

void CollectionInstance::reregisterEvents(const QString& eventuid) {
    QMap<QString, QVariantMap> eventdataCache;
    if (eventuid.size()) {
		eventdataCache = m_eventdataCache;
        ServiceController::ServiceStruct* service = m_servicecontroller->service(eventuid);
        if (!service) {
            qWarning() << "Event"<<eventuid<<"not found for collection"<<collectionuid;
            return;
        }
        eventdataCache.insert(ServiceType::uniqueID(service->data), service->data);
    } else { // new events data cache
        QSet<QString>::iterator i = eventids.begin();
        for (;i!=eventids.end();++i) {
            ServiceController::ServiceStruct* service = m_servicecontroller->service(*i);
            if (!service) {
                qWarning() << "Event"<<*i<<"not found for collection"<<collectionuid;
                return;
            }
            eventdataCache.insert(ServiceType::uniqueID(service->data), service->data);
        }
    }

    if (eventdataCache == m_eventdataCache) return;

    QMutableMapIterator<QString, QVariantMap> oldit(m_eventdataCache);
    while (oldit.hasNext()) {
        oldit.next();
        QVariantMap data = oldit.value();
        QString eventuid = ServiceType::uniqueID(data);

        QMap<QString,QVariantMap>::const_iterator i = eventdataCache.find(eventuid);
        // nicht in neuen events gefunden, nur unregisteren
        if (i==eventdataCache.end() || i.value() != data) {
            unregisterEvent(data);
            oldit.remove();
        }
    }

    QMap<QString,QVariantMap>::const_iterator i = eventdataCache.begin();
    for (;i!=eventdataCache.end();++i) {
        if (m_eventdataCache.find(ServiceType::uniqueID(i.value())) == m_eventdataCache.end()) {
            registerEvent(i.value());
        }
    }
    m_eventdataCache = eventdataCache;
}

void CollectionInstance::clone() {
    QVariantMap newcollection = m_servicecontroller->cloneService(collectionuid);
    if (newcollection.isEmpty()) return;
    newcollection[QLatin1String("name")] = QString(newcollection[QLatin1String("name")].toString() +  QLatin1String(" (copy)"));
    newcollection[QLatin1String("actions")] = QVariantList();
    newcollection[QLatin1String("conditions")] = QVariantList();
    newcollection[QLatin1String("events")] = QVariantList();
    m_servicecontroller->changeService(newcollection, QString());
    QString newcollectionuid = ServiceType::uniqueID(newcollection);

    foreach(QString uid, eventids) {
        QVariantMap newservice = m_servicecontroller->cloneService(uid);
        ServiceType::setToCollection(newservice, newcollectionuid);
        m_servicecontroller->changeService(newservice, QString());
    }
    foreach(QString uid, actionids) {
        QVariantMap newservice = m_servicecontroller->cloneService(uid);
        ServiceType::setToCollection(newservice, newcollectionuid);
        m_servicecontroller->changeService(newservice, QString());
    }
    foreach(QString uid, conditionids) {
        QVariantMap newservice = m_servicecontroller->cloneService(uid);
        ServiceType::setToCollection(newservice, newcollectionuid);
        m_servicecontroller->changeService(newservice, QString());
    }
}
