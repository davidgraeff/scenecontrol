#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "services/actorcurtain.h"
#include "services/conditionled.h"
#include "services/conditioncurtain.h"
#include "services/actorledname.h"
#include "services/actorled.h"
#include "services_server/actorcurtainServer.h"
#include "services_server/actorledServer.h"
#include "services_server/actorlednameServer.h"
#include "services_server/conditioncurtainServer.h"
#include "services_server/conditionledServer.h"
#include "controller.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
  m_base = new myPlugin();
  m_Controller = new Controller(this);
  connect(m_Controller,SIGNAL(stateChanged(AbstractStateTracker*)),SIGNAL(stateChanged(AbstractStateTracker*)));
  connect(m_Controller,SIGNAL(dataLoadingComplete()),SLOT(dataLoadingComplete()));
}

myPluginExecute::~myPluginExecute() {
  //delete m_base;
  delete m_Controller;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorCurtain::staticMetaObject.className()) {
        return new ActorCurtainServer((ActorCurtain*)service, this);
    } else if (idb == ActorLed::staticMetaObject.className()) {
        return new ActorLedServer((ActorLed*)service, this);
    } else if (idb == ActorLedName::staticMetaObject.className()) {
        return new ActorLedNameServer((ActorLedName*)service, this);
    } else if (idb == ConditionCurtain::staticMetaObject.className()) {
        return new ConditionCurtainServer((ConditionCurtain*)service, this);
    } else if (idb == ConditionLed::staticMetaObject.className()) {
        return new ConditionLedServer((ConditionLed*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    return m_Controller->getStateTracker();
}
void myPluginExecute::dataLoadingComplete() {
    emit pluginLoadingComplete(this);
}
