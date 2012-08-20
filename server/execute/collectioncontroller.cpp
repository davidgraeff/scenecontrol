
#include "paths.h"
#include "collectioncontroller.h"
#include "socket.h"
#include "runningcollection.h"
#include "shared/jsondocuments/scenedocument.h"
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
        RunningCollection* run = m_cachedCollections[foundIndex];
        m_runningCollections.insert(collectionid, run);
        updateListOfRunningCollections();
        run->start();
    } else
        Database::instance()->requestDataOfCollection(collectionid);
}

void CollectionController::runningCollectionFinished(const QString& collectionid)
{
    // Remove collection from list of running collections
    RunningCollection* run = m_runningCollections.take(collectionid);
    // Remove all instances from the cache and add another one at the end
    m_cachedCollections.removeAll(run);
    m_cachedCollections.append(run);
    // if cache size > running collections or 1: trim down
    while (m_cachedCollections.size() > m_runningCollections.size()) {
	run = m_cachedCollections.takeFirst();
	Q_ASSERT(!m_runningCollections.contains(collectionid));
	delete run;
    }
    updateListOfRunningCollections();
}

void CollectionController::dataOfCollection(const QString& collectionid, const QList< QVariantMap >& services)
{
    {
      RunningCollection* old = m_runningCollections.take(collectionid);
      m_cachedCollections.removeAll(old);
      delete old;
    }
    RunningCollection* run = new RunningCollection(collectionid, services);
    connect(run, SIGNAL(runningCollectionFinished(QString)), SLOT(runningCollectionFinished(QString)));
    m_runningCollections.insert(collectionid, run);
    updateListOfRunningCollections();
    run->start();
}

void CollectionController::updateListOfRunningCollections()
{
    SceneDocument doc = SceneDocument::createNotification("collection.running");
    QVariantList list;
    QList<QString> orig = m_runningCollections.keys();
    for (int i=0;i<orig.size(); ++i) {
        list.append(orig[i]);
    }
    doc.setData("running",list);
    doc.setPluginid("CollectionController");
    Socket::instance()->propagateProperty(doc.getData(), -1);
}
