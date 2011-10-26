#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"

#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "config.h"
#include "plugincontroller.h"
#include "websocket.h"
#include <QDebug>
#define __FUNCTION__ __FUNCTION__

#include "collectioncontroller.h"
#include "couchdb.h"
#include <shared/abstractserver_propertycontroller.h>

RunningCollection::RunningCollection(const QVariantList& actions, const QString& collectionid): QObject(), m_collectionid(collectionid), m_lasttime(0)
{
    for ( int i=0;i<actions.size();++i ) {
        QVariantMap actiondata = actions.value(i).toMap().value ( QLatin1String ( "value" ) ).toMap();
        m_timetable.insert (actiondata.value(QLatin1String("delay_"), 0).toInt(), actiondata);
    }
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    m_timer.setSingleShot(true);
}

void RunningCollection::start()
{
    timeout();
}

void RunningCollection::timeout()
{
    QMap<int, QVariantMap>::const_iterator lowerBound = m_timetable.lowerBound(m_lasttime);
    if (lowerBound == m_timetable.constEnd()) {
        emit runningCollectionFinished(m_collectionid);
        return;
    }
    QMap<int, QVariantMap>::const_iterator upperBound = m_timetable.upperBound(m_lasttime);

    while (lowerBound != upperBound) {
        emit runningCollectionAction(lowerBound.value());
        ++lowerBound;
    }

    // remove all entries belonging to this time slot
    m_lasttime = lowerBound.key();
    m_timetable.remove(lowerBound.key());

    // restart timer for next time slot
    lowerBound = m_timetable.lowerBound(m_lasttime);
    if (lowerBound == m_timetable.constEnd()) {
        emit runningCollectionFinished(m_collectionid);
        return;
    }
    m_timer.start(lowerBound.key() - m_lasttime);
}

CollectionController::CollectionController () : m_plugincontroller ( 0 ) {}

CollectionController::~CollectionController() {}

void CollectionController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void CollectionController::pluginEventTriggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    Q_UNUSED ( event_id );
    CouchDB::instance()->requestActionsOfCollection(destination_collectionuid);
}

void CollectionController::requestExecution(const QVariantMap& data, int sessionid) {
    if ( !ServiceID::isExecutable ( data ) || sessionid == -1) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }

    executeplugin->execute ( data, sessionid );
}

void CollectionController::runningCollectionAction(const QVariantMap& actiondata)
{
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( actiondata ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<actiondata;
        return;
    }

    executeplugin->execute ( actiondata, -1 );
}

void CollectionController::runningCollectionFinished(const QString& collectionid)
{
    RunningCollection* run = m_runningCollections.take(collectionid);
    if (run)
      run->deleteLater();
    updateListOfRunningCollections();
}

void CollectionController::actionsOfCollection(const QVariantList& actions, const QString& collectionid)
{
    delete m_runningCollections.take(collectionid);
    RunningCollection* run = new RunningCollection(actions, collectionid);
    connect(run, SIGNAL(runningCollectionAction(QVariantMap)), SLOT(runningCollectionAction(QVariantMap)));
    connect(run, SIGNAL(runningCollectionFinished(QString)), SLOT(runningCollectionFinished(QString)));
    m_runningCollections.insert(collectionid, run);
    updateListOfRunningCollections();

    run->start();
}

void CollectionController::updateListOfRunningCollections()
{
    ServiceCreation data = ServiceCreation::createNotification(PLUGIN_ID, "collection.running");
    QVariantList list;
    QList<QString> orig = m_runningCollections.keys();
    for (int i=0;i<orig.size(); ++i) {
      list.append(orig[i]);
    }
    data.setData("running",list);
    m_serverPropertyController->pluginPropertyChanged(data.getData(), -1, PLUGIN_ID);
}

void CollectionController::pluginRequestExecution ( const QVariantMap& data, const char* pluginid ) {
    if ( !ServiceID::isExecutable ( data ) && !ServiceID::isAction ( data ) ) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }
    if (pluginid)
        qDebug() << "Plugin" << pluginid << "executes action";

    executeplugin->execute ( data, -1 );
}

void CollectionController::execute(const QVariantMap& data, int sessionid) {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "collection.execute" ) ) {
        CouchDB::instance()->requestActionsOfCollection(data.value(QLatin1String("collectionid")).toString());
    } else if ( ServiceID::isMethod(data, "collection.stop" ) ) {
        runningCollectionFinished(data.value(QLatin1String("collectionid")).toString());
    }
}
