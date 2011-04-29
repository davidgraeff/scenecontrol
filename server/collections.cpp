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

#include "collections.h"
#include <QDebug>
#include <QCoreApplication>
#include "servicecontroller.h"
#include "boolstuff/BoolExprParser.h"

Collections::Collections() : m_servicecontroller(0) {
}

Collections::~Collections() {
    qDeleteAll(m_collections);
    m_collections.clear();
}

QList< QVariantMap > Collections::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    return l;
}

bool Collections::condition(const QVariantMap& data, const QString& sessionid)  {
    Q_UNUSED(data);
    Q_UNUSED(sessionid);
    return false;
}

void Collections::event_changed(const QVariantMap& data, const QString& sessionid)  {
    Q_UNUSED(data);
    Q_UNUSED(sessionid);
}

void Collections::execute(const QVariantMap& data, const QString& sessionid)  {
    Q_UNUSED(sessionid);
    if (ServiceID::isId(data,"executecollection")) {
        CollectionInstance* instance = m_collections.value(DATA("collectionid"));
        if (instance && instance->enabled) instance->startExecution();
    } else if (ServiceID::isId(data,"stopcollection")) {
        CollectionInstance* instance = m_collections.value(DATA("collectionid"));
        if (instance) instance->stop();
    }
}

void Collections::initialize() {
}

void Collections::clear() {
}

void Collections::addCollection(const QVariantMap& data) {

    CollectionInstance* instance = new CollectionInstance();
    m_collections.insert(ServiceType::uniqueID(data), instance);
    instance->enabled = BOOLDATA("enabled");
    convertVariantToIntStringMap(MAP("actions"), instance->actionids);
    convertVariantToStringSet(LIST("conditions"), instance->conditionids);
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
    convertVariantToStringSet(LIST("events"), instance->eventids);
    connect(instance,SIGNAL(executeService(QString, QString)),SIGNAL(instanceExecute(QString, QString)));
}

void Collections::dataSync(const QVariantMap& data, const QString& sessionid) {
    Q_UNUSED(sessionid);
    if (!ServiceType::isCollection(data)) return;
    delete m_collections.take(ServiceType::uniqueID(data));
    addCollection(data);
}

void Collections::dataReady() {
    // clear
    qDeleteAll(m_collections);
    m_collections.clear();

    // get collections
    const QMap< QString, ServiceController::ServiceStruct* > services = m_servicecontroller->valid_services();
    foreach (ServiceController::ServiceStruct* service, services) {
        const QVariantMap data = service->data;
        if (ServiceType::isCollection(data)) {
            addCollection(data);
        }
    }
}

void Collections::eventTriggered(const QString& uid) {
    foreach(CollectionInstance* instance, m_collections) {
        if (instance->enabled && instance->eventids.contains(uid)) {
            // conditions
            bool ok = true;
            foreach(QString uid, instance->conditionids) {
                ServiceController::ServiceStruct* s =  m_servicecontroller->service(uid);
                if (!s) continue;
                if (!s->plugin->condition(s->data, QString())) {
                    ok = false;
                    break;
                }
            }
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

            if (!ok) continue;

            // execute
            instance->startExecution();
        }
    }
}

void Collections::convertVariantToStringSet(const QVariantList& source, QSet<QString>& destination) {
    foreach (QVariant v, source) {
        destination.insert(v.toString());
    }
}

void Collections::convertVariantToIntStringMap(const QVariantMap& source, QMap< int, QString >& destination) {
    QVariantMap::const_iterator i = source.constBegin();
    for (;i!=source.constEnd();++i)
        destination.insertMulti(i.key().toInt(), i.value().toString());
}

CollectionInstance::CollectionInstance() : conditionlinks(0) {
    m_executionTimer.setSingleShot ( true );
    connect ( &m_executionTimer, SIGNAL ( timeout() ),SLOT ( executiontimeout() ) );
}

CollectionInstance::~CollectionInstance() {
    delete conditionlinks;
}

void CollectionInstance::startExecution() {
    m_executionTimer.stop();
    QMap< int, QString >::iterator it = actionids.lowerBound(0);
    if (it == actionids.end()) return;
    m_currenttime = it.key();
    m_executionTimer.start ( 1000*m_currenttime );
    m_currenttime++;

}

void CollectionInstance::executiontimeout()
{
    const QList<QString> uids = actionids.values();
    foreach (QString uid, uids) {
        emit executeService(uid, QString());
    }

    QMap< int, QString >::iterator it = actionids.lowerBound(m_currenttime);
    if (it == actionids.end()) return;
    m_currenttime = it.key();
    m_executionTimer.start ( 1000*m_currenttime );
    m_currenttime++;
}

void CollectionInstance::stop() {
    m_executionTimer.stop();
}
