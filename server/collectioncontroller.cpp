
#include "paths.h"
#include "config.h"
#include <QDebug>
#include "collectioncontroller.h"
#include "database.h"
#include <shared/pluginservicehelper.h>
#include "plugincontroller.h"
#include "pluginprocess.h"
#include "socket.h"
#include "runningcollection.h"
#define __FUNCTION__ __FUNCTION__

static CollectionController* collectioncontroller_instance = 0;

CollectionController* CollectionController::instance() {
    if (!collectioncontroller_instance) {
        collectioncontroller_instance = new CollectionController();
    }
    return collectioncontroller_instance;
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

void CollectionController::requestExecution(const QVariantMap& data, int sessionid) {
    if ( !ServiceData::checkType ( data, ServiceData::TypeExecution )) return;
    // Special case: all properties are requested, handle this immediatelly.
    if (ServiceData::pluginid(data)==QLatin1String("server") && sessionid != -1) {
        if (ServiceData::isMethod(data, "requestAllProperties"))
            PluginController::instance()->requestAllProperties(sessionid);
        else if (ServiceData::isMethod(data, "runcollection"))
            requestExecutionByCollectionId(ServiceData::collectionid(data));
        else if (ServiceData::isMethod(data, "version")) {
            ServiceData s = ServiceData::createNotification("version");
            s.setData("version", QLatin1String(ABOUT_VERSION));
            s.setPluginid("server");
            Socket::instance()->propagateProperty(s.getData(), sessionid);
        }
        return;
    }
    // Look for a plugin that fits "data"
    PluginCommunication* plugin = PluginController::instance()->getPlugin ( ServiceData::pluginid ( data ) );
    if ( !plugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data << sessionid;
        return;
    }

    plugin->callQtSlot ( data, QByteArray(), sessionid );
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

void CollectionController::dataOfCollection(const QList< QVariantMap >& actions, const QList< QVariantMap >& conditions, const QString& collectionid)
{
    delete m_runningCollections.take(collectionid);
    RunningCollection* run = new RunningCollection(actions, conditions, collectionid);
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
