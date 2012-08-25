
#include "paths.h"
#include "scenecontroller.h"
#include "controlsocket/socket.h"
#include "runningscene.h"
#include "shared/jsondocuments/scenedocument.h"
#include "libdatastorage/datastorage.h"
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

void CollectionController::requestExecutionByCollectionId ( const QString& sceneid ) {
    int foundIndex=-1;
    for (int i=0;i<m_cachedCollections.size();++i)
        if (m_cachedCollections[i]->sceneid() == sceneid) {
            foundIndex = i;
            break;
        }
    if (foundIndex!=-1) {
        RunningCollection* run = m_cachedCollections[foundIndex];
        m_runningCollections.insert(sceneid, run);
        updateListOfRunningCollections();
        run->start();
		return;
    }
    // request conditions, actions
	{
      RunningCollection* old = m_runningCollections.take(sceneid);
      m_cachedCollections.removeAll(old);
      delete old;
    }
    SceneDocument filter;
	filter.setSceneid(sceneid);
    QList< SceneDocument* > documents = DataStorage::instance()->requestAllOfType(SceneDocument::TypeCondition, filter.getData());
    documents += DataStorage::instance()->requestAllOfType(SceneDocument::TypeAction, filter.getData());
    RunningCollection* run = new RunningCollection(sceneid, documents);
    connect(run, SIGNAL(runningCollectionFinished(QString)), SLOT(runningCollectionFinished(QString)));
    m_runningCollections.insert(sceneid, run);
    updateListOfRunningCollections();
    run->start();
}

void CollectionController::runningCollectionFinished(const QString& sceneid)
{
    // Remove collection from list of running collections
    RunningCollection* run = m_runningCollections.take(sceneid);
    // Remove all instances from the cache and add another one at the end
    m_cachedCollections.removeAll(run);
    m_cachedCollections.append(run);
    // if cache size > running collections or 1: trim down
    while (m_cachedCollections.size() > m_runningCollections.size()) {
	run = m_cachedCollections.takeFirst();
	Q_ASSERT(!m_runningCollections.contains(sceneid));
	delete run;
    }
    updateListOfRunningCollections();
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
    doc.setComponentID(QLatin1String("CollectionController"));
    Socket::instance()->sendToClients(doc.getjson(), -1);
}
