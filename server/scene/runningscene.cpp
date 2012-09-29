#include "runningscene.h"
#include "plugins/plugincontroller.h"
#include "shared/jsondocuments/scenedocument.h"
#include <plugins/pluginprocess.h>


RunningCollection::RunningCollection(const QString& collectionid, const QList< SceneDocument* >& services):
        QObject(), m_collectionid(collectionid), m_lasttime(0), m_conditionok(0)
{
    // Add all actions ("actions" list contains (key,value) pairs. The value is the actuall action data)
    // into "m_timetable" depending on their start delay.
    for ( int i=0;i<services.size();++i ) { // action
        // Extract data and delay time
        SceneDocument* doc = services[i];
        if (doc->checkType(SceneDocument::TypeAction)) {
            const int delay = doc->actiondelay();
            // Get the right plugin process
            PluginProcess* plugin = PluginController::instance()->getPlugin (doc->componentUniqueID());
            if ( !plugin ) {
                qWarning() <<"No plugin for action found:"<<doc->getjson();
                continue;
            }

            m_timetable.insert (delay, dataWithPlugin(plugin, doc));
        } else if (doc->checkType(SceneDocument::TypeCondition)) { // condition
            // Get the right plugin process
            PluginProcess* plugin = PluginController::instance()->getPlugin (doc->componentUniqueID());
            if ( !plugin ) {
				qWarning() <<"No plugin for condition found:"<<doc->getjson();
                continue;
            }

            m_conditions.append (dataWithPlugin(plugin, doc));
            connect(plugin,SIGNAL(qtSlotResponse(QVariant,QByteArray,QString,QString)),
                    SLOT(qtSlotResponse(QVariant,QByteArray,QString,QString)));

        }
    }

    // prepare timer
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeoutNextAction()));
    m_timer.setSingleShot(true);
}

void RunningCollection::start()
{
    m_runningtimetable = m_timetable;
    m_lasttime = 0;
    // only evaluate condition once if last check was ok
    if (m_conditionok == m_conditions.size()) {
        timeoutNextAction();
        return;
    }
    m_conditionok = 0;
    // Check conditions
    for (int i=0; i < m_conditions.size(); ++i) {
        m_conditions[i].plugin->callQtSlot ( m_conditions[i].doc->getData(), QByteArray::number(i) );
    }
    if (m_conditions.isEmpty()) {
        timeoutNextAction();
    } else
        qDebug() <<"Start with cond" << m_collectionid << "Ok:" << m_conditionok << "size:" << m_conditions.size();
}

void RunningCollection::qtSlotResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid) {
    if (!response.canConvert(QVariant::Bool)) {
        qWarning() << "Condition failed." << responseid << pluginid << instanceid << "Not a boolean response" << response;
        return;
    }
    if (!response.toBool()) {
        qDebug() << "Not executing" << m_collectionid << "; Condition" << responseid << "false";
        return;
    }
    ++m_conditionok;
    conditionResponse();
}

void RunningCollection::conditionResponse(bool timeout)
{
    if (timeout || m_conditionok>=m_conditions.size()) {
        // Start executing
        timeoutNextAction();
        m_conditionok = m_conditions.size();
    }
}

void RunningCollection::timeoutNextAction()
{
    QMap<int, dataWithPlugin>::const_iterator lowerBound = m_runningtimetable.lowerBound(m_lasttime);
    if (lowerBound == m_runningtimetable.constEnd()) {
        emit runningCollectionFinished(m_collectionid);
        return;
    }
    QMap<int, dataWithPlugin>::const_iterator upperBound = m_runningtimetable.upperBound(m_lasttime);


    while (lowerBound != upperBound) {
        const dataWithPlugin& dp = lowerBound.value();
        dp.plugin->callQtSlot ( *dp.doc );
        ++lowerBound;
    }


    // remove all entries belonging to this time slot
    m_lasttime = lowerBound.key();
    m_runningtimetable.remove(lowerBound.key());

    // restart timer for next time slot
    lowerBound = m_runningtimetable.lowerBound(m_lasttime);
    if (lowerBound == m_runningtimetable.constEnd()) {
        emit runningCollectionFinished(m_collectionid);
        return;
    }
    m_timer.start(lowerBound.key() - m_lasttime);
}

QString RunningCollection::sceneid() const
{
    return m_collectionid;
}
