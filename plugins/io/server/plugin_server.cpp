#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "iocontroller.h"
#include "services/actorpin.h"
#include "services/actorpinname.h"
#include "services/conditionpin.h"
#include "services_server/actorpinServer.h"
#include "services_server/actorpinnameServer.h"
#include "services_server/conditionpinServer.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
  m_base = new myPlugin();
  m_IOController = new IOController();
  connect(m_IOController,SIGNAL(stateChanged(AbstractStateTracker*)),SIGNAL(stateChanged(AbstractStateTracker*)));
}

myPluginExecute::~myPluginExecute() {
  //delete m_base;
  delete m_IOController;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorPin::staticMetaObject.className()) {
        return new ActorPinServer((ActorPin*)service, this);
    } else if (idb == ActorPinName::staticMetaObject.className()) {
        return new ActorPinNameServer((ActorPinName*)service, this);
    } else if (idb == ConditionPin::staticMetaObject.className()) {
        return new ConditionPinServer((ConditionPin*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    return m_IOController->getStateTracker();
}
