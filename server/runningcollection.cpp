#include "runningcollection.h"
#include "plugincontroller.h"
#include <shared/pluginservicehelper.h>
#include "pluginprocess.h"


RunningCollection::RunningCollection(const QList< QVariantMap >& actions, const QList< QVariantMap >& conditions, const QString& collectionid):
        QObject(), m_collectionid(collectionid), m_lasttime(0), m_conditionok(0)
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
	// only evaluate condition once if last check was ok
	if (m_conditionok == m_conditions.size()) {
		timeoutNextAction();
		return;
	}
	m_lasttime = 0;
	m_runningtimetable = m_timetable;
    m_conditionok = 0;
    // Check conditions
    for (int i=0; i < m_conditions.size(); ++i) {
		m_conditions[i].data.insert(QLatin1String("responseid_"), ServiceData::id(m_conditions[i].data));
        m_conditions[i].plugin->callQtSlot ( m_conditions[i].data, QByteArray::number(i) );
    }
    if (m_conditions.isEmpty()) {
        timeoutNextAction();
	} else
		qDebug() <<"Start with cond" << m_collectionid << "Ok:" << m_conditionok << "size:" << m_conditions.size();
}

void RunningCollection::qtSlotResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid) {
    if (!response.canConvert(QVariant::Bool)) {
        qWarning() << "Condition failed." << responseid << pluginid << "Not a boolean response" << response;
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
        dp.plugin->callQtSlot ( dp.data );
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
