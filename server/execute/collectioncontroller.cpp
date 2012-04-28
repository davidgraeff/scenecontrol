
#include "paths.h"
#include "collectioncontroller.h"
#include "socket.h"
#include "runningcollection.h"
#include "libdatabase/servicedata.h"
#include "libdatabase/database.h"
#include <QDebug>
#define __FUNCTION__ __FUNCTION__

static CollectionController* ExecuteRequest_instance = 0;

CollectionController* CollectionController::instance() {
    if (!ExecuteRequest_instance) {
        ExecuteRequest_instance = new CollectionController();
    }
    return ExecuteRequest_instance;
}

CollectionController::CollectionController () {}

CollectionController::~CollectionController() {}

void CollectionController::requestExecutionByCollectionId ( const QString& collectionid ) {
    int foundIndex=-1;
    for (int i=0;i<m_cachedCollections.size();++i)
        if (m_cachedCollections[i]->id() == collectionid) {
            foundIndex = i;
            break;
        }
    if (foundIndex!=-1) {
  qDebug()<<__FUNCTION__;
        RunningCollection* run = m_cachedCollections[foundIndex];
        m_runningCollections.insert(collectionid, run);
        updateListOfRunningCollections();
  qDebug()<<__FUNCTION__;
        run->start();
  qDebug()<<__FUNCTION__;
    } else
        Database::instance()->requestDataOfCollection(collectionid);
}

void CollectionController::runningCollectionFinished(const QString& collectionid)
{
  qDebug()<<__FUNCTION__;
    // Remove collection from list of running collections
    RunningCollection* run = m_runningCollections.take(collectionid);
    // Remove all instances from the cache and add another one at the end
  qDebug()<<__FUNCTION__;
    m_cachedCollections.removeAll(run);
    m_cachedCollections.append(run);
    // if cache size > running collections or 1: trim down
  qDebug()<<__FUNCTION__;
    while (qMax<int>(1, m_cachedCollections.size()) > m_runningCollections.size()) {
	run = m_cachedCollections.takeFirst();
	Q_ASSERT(!m_runningCollections.contains(collectionid));
	delete run;
    }
  qDebug()<<__FUNCTION__;
    updateListOfRunningCollections();
  qDebug()<<__FUNCTION__;
}

void CollectionController::dataOfCollection(const QString& collectionid, const QList< QVariantMap >& services)
{
  qDebug()<<__FUNCTION__;
    delete m_runningCollections.take(collectionid);
    RunningCollection* run = new RunningCollection(collectionid, services);
  qDebug()<<__FUNCTION__;
    connect(run, SIGNAL(runningCollectionFinished(QString)), SLOT(runningCollectionFinished(QString)));
    m_runningCollections.insert(collectionid, run);
  qDebug()<<__FUNCTION__;
    updateListOfRunningCollections();
  qDebug()<<__FUNCTION__;
    run->start();
}

void CollectionController::updateListOfRunningCollections()
{
    ServiceData data = ServiceData::createNotification("collection.running");
    QVariantList list;
    QList<QString> orig = m_runningCollections.keys();
    for (int i=0;i<orig.size(); ++i) {
        list.append(orig[i]);
    }
    data.setData("running",list);
    data.setPluginid("CollectionController");
    Socket::instance()->propagateProperty(data.getData(), -1);
}
