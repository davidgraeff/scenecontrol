
#include "paths.h"
#include "config.h"
#include "collectioncontroller.h"
#include "socket.h"
#include "runningcollection.h"
#include "shared/pluginservicehelper.h"
#include "shared/database.h"
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
    RunningCollection* run = m_runningCollections.take(collectionid);
    if (run && !m_cachedCollections.contains(run)) {
        if (m_cachedCollections.size()>1) {
            delete m_cachedCollections.first();
            m_cachedCollections.pop_front();
        }
        m_cachedCollections.push_back(run);
    }
    updateListOfRunningCollections();
}

void CollectionController::dataOfCollection(const QString& collectionid, const QList< QVariantMap >& services)
{
    delete m_runningCollections.take(collectionid);
    RunningCollection* run = new RunningCollection(collectionid, services);
    connect(run, SIGNAL(runningCollectionFinished(QString)), SLOT(runningCollectionFinished(QString)));
    m_runningCollections.insert(collectionid, run);
    updateListOfRunningCollections();
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
