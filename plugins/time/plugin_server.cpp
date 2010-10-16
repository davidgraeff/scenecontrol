#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "../server/executeservice.h"
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
