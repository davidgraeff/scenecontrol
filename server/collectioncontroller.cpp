
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "config.h"
#include "plugincontroller.h"
#include "socket.h"
#include <QDebug>
#define __FUNCTION__ __FUNCTION__

#include "shared/pluginservicehelper.h"
#include "collectioncontroller.h"
#include "couchdb.h"
#include "pluginprocess.h"

RunningCollection::RunningCollection(const QList< QVariantMap >& actions, const QList< QVariantMap >& conditions, const QString& collectionid):
        QObject(), m_collectionid(collectionid), m_lasttime(0)
{
    // Add all actions ("actions" list contains (key,value) pairs. The value is the actuall action data)
    // into "m_timetable" depending on their start delay.
    for ( int i=0;i<actions.size();++i ) {
        // Extract data and delay time
        const QVariantMap& actiondata = actions[i];
        const int delay = actiondata.value(QLatin1String("delay_"), 0).toInt();
        // Get the right plugin process
        PluginCommunication* plugin = PluginController::instance()->getPlugin ( ServiceData::pluginid ( actiondata ) );
        if ( !plugin ) {
            qWarning() <<"No plugin for action found:"<<actiondata;
            continue;
        }

        m_timetable.insert (delay, dataWithPlugin(plugin, actiondata));
    }
    // Add conditions
    for ( int i=0;i<conditions.size();++i ) {
        // Extract data and delay time
        const QVariantMap& conditiondata = conditions[i];
        // Get the right plugin process
        PluginCommunication* plugin = PluginController::instance()->getPlugin ( ServiceData::pluginid ( conditiondata ) );
        if ( !plugin ) {
            qWarning() <<"No plugin for condition found:"<<conditiondata;
            continue;
        }

        m_conditions.append (dataWithPlugin(plugin, conditiondata));
        connect(plugin,SIGNAL(qtSlotResponse(QVariant,QByteArray,QString)),
                SLOT(qtSlotResponse(QVariant,QByteArray,QString)));
    }
    // prepare timer
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeoutNextAction()));
    m_timer.setSingleShot(true);
}

void RunningCollection::start()
{
    m_conditionok = 0;
    // Check conditions
    for (int i=0; i < m_conditions.size(); ++i) {
        m_conditions[i].plugin->callQtSlot ( m_conditions[i].data, QByteArray::number(i) );
    }
    if (m_conditions.isEmpty()) {
        timeoutNextAction();
    }
    qDebug() << m_collectionid << __FUNCTION__ << m_conditionok << m_conditions.size();
}

void RunningCollection::qtSlotResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid) {
    qDebug() << m_collectionid << __FUNCTION__ << m_conditionok;
    Q_UNUSED(responseid);
    if (!response.canConvert(QVariant::Bool)) {
        qWarning() << "Condition check failed. Return value not a boolean" << pluginid << response;
        return;
    }
    if (!response.toBool()) {
        qDebug() << "Condition false. Not executing" << m_collectionid;
        return;
    }
    ++m_conditionok;
    conditionResponse();
}

void RunningCollection::conditionResponse(bool timeout)
{
    if (timeout || m_conditionok>=m_conditions.size())
        // Start executing
        timeoutNextAction();
}

void RunningCollection::timeoutNextAction()
{
    QMap<int, dataWithPlugin>::const_iterator lowerBound = m_timetable.lowerBound(m_lasttime);
    if (lowerBound == m_timetable.constEnd()) {
        emit runningCollectionFinished(m_collectionid);
        return;
    }
    QMap<int, dataWithPlugin>::const_iterator upperBound = m_timetable.upperBound(m_lasttime);


    while (lowerBound != upperBound) {
        const dataWithPlugin& dp = lowerBound.value();
        dp.plugin->callQtSlot ( dp.data );
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
    CouchDB::instance()->requestDataOfCollection(collectionid);
}

void CollectionController::requestExecution(const QVariantMap& data, int sessionid) {
    if ( !ServiceData::checkType ( data, ServiceData::TypeExecution )) return;
    if (ServiceData::pluginid(data)==QLatin1String("server") && ServiceData::isMethod(data, "requestAllProperties") && sessionid != -1) {
        PluginController::instance()->requestAllProperties(sessionid);
        return;
    }
    PluginCommunication* plugin = PluginController::instance()->getPlugin ( ServiceData::pluginid ( data ) );
    if ( !plugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data << sessionid;
        return;
    }

    plugin->callQtSlot ( data );
}

void CollectionController::runningCollectionFinished(const QString& collectionid)
{
    RunningCollection* run = m_runningCollections.take(collectionid);
    if (run)
        run->deleteLater();
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
