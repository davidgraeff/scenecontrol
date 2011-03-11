#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "services/eventperiodic.h"
#include "services/eventdatetime.h"
#include "services/conditiontimespan.h"
#include "services_server/eventperiodicServer.h"
#include "services_server/eventdatetimeServer.h"
#include "services_server/conditiontimespanServer.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
  m_base = new myPlugin();
}

myPluginExecute::~myPluginExecute() {
  //delete m_base;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == EventPeriodic::staticMetaObject.className()) {
        return new EventPeriodicServer((EventPeriodic*)service, this);
    } else if (idb == EventDateTime::staticMetaObject.className()) {
        return new EventDateTimeServer((EventDateTime*)service, this);
    } else if (idb == ConditionTimespan::staticMetaObject.className()) {
        return new ConditionTimespanServer((ConditionTimespan*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    return a;
}



#include "eventperiodicServer.h"
#include <QDebug>
#include <services/eventperiodic.h>
#include <QDateTime>
#include "server/plugin_server.h"

void EventPeriodicServer::timeout(bool aftersync)
{
    // timer triggered this timeout
    if (!aftersync && !m_aftertrigger)  {
        emit trigger();
        // get back in 2 seconds to generate the new periodic alarm
        m_aftertrigger = true;
        m_timer.start(2000);
        return;
    }
    m_aftertrigger = false;

	EventPeriodic* base = service<EventPeriodic>();
	QTime m_time = QDateTime::fromString(base->time(),Qt::ISODate).time();
	m_time.setHMS(m_time.hour(),m_time.minute(),0);
	QDateTime datetime = QDateTime::currentDateTime();
    datetime.setTime ( m_time );
    int dow = QDate::currentDate().dayOfWeek() - 1;
    int offsetdays = 0;

    // If it is too late for the alarm today then
    // try with the next weekday
    if ( QTime::currentTime() > m_time )
    {
        dow = (dow+1) % 7;
        ++offsetdays;
    }

    // Search for the next activated weekday
    for (; offsetdays < 8; ++offsetdays )
    {
        if ( base->m_days[dow] ) break;
        dow = (dow+1) % 7;
    }

    if ( offsetdays >= 8 ) {
        return;
		qWarning()<<"Periodic alarm: Failure with weekday offset"<<offsetdays<<datetime.toString(Qt::DefaultLocaleShortDate);
    }

    const int sec = QDateTime::currentDateTime().secsTo (datetime.addDays ( offsetdays ));
    m_timer.start(sec*1000);
	qDebug() << "Periodic alarm: " << datetime.addDays ( offsetdays ).toString(Qt::DefaultLocaleShortDate);
}
EventPeriodicServer::EventPeriodicServer(EventPeriodic* base, myPluginExecute*, QObject* parent) : ExecuteService(base, parent), m_aftertrigger(false)
{
    m_timer.setSingleShot(true);
    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}
bool EventPeriodicServer::checkcondition() {
    return true;
}
void EventPeriodicServer::dataUpdate() {
    timeout(true);
}
void EventPeriodicServer::execute() {}

void EventPeriodicServer::nameUpdate() {
    EventPeriodic* base = service<EventPeriodic>();
    QString days;
    for (int i=0;i<7;++i) {
        if (base->days() & (1<<i)) {
            days.append(QDate::shortDayName(i+1));
            days.append(QLatin1String(","));
        }
    }
    days.chop(1);
	const QString time = QDateTime::fromString(base->time(),Qt::ISODate).time().toString(Qt::DefaultLocaleShortDate);
    base->setString(tr("Periodisch um %1\nan: %2").arg(time).arg(days));
}



EventDateTimeServer::EventDateTimeServer(EventDateTime* base, myPluginExecute* , QObject* parent)
        : ExecuteService(base, parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

void EventDateTimeServer::dataUpdate()
{
    timeout();
}

void EventDateTimeServer::timeout()
{
  EventDateTime* base = service<EventDateTime>();
    const int sec = QDateTime::currentDateTime().secsTo (QDateTime::fromString(base->datetime(),Qt::ISODate));

    if (sec > 86400) {
        qDebug() << "One-time alarm: Armed (next check only)";
        m_timer.start(86400*1000);
    } else if (sec > 10) {
		qDebug() << "One-time alarm: Armed" << sec;
        m_timer.start(sec*1000);
    } else if (sec > -10 && sec < 10) {
        emit trigger();
    }
}

bool ConditionTimespanServer::checkcondition() {
  const ConditionTimespan* base = service<ConditionTimespan>();
  QTime m_lower = QTime::fromString(base->lower(),Qt::ISODate);
  QTime m_upper = QTime::fromString(base->upper(),Qt::ISODate);
    if (QTime::currentTime() < m_lower) return false;
    if (QTime::currentTime() > m_upper) return false;
    return true;
}


