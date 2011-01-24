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
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
  m_base = new myPlugin();
  m_IOController = new IOController(this);
  connect(m_IOController,SIGNAL(stateChanged(AbstractStateTracker*)),SIGNAL(stateChanged(AbstractStateTracker*)));
  connect(m_IOController,SIGNAL(dataLoadingComplete()),SLOT(dataLoadingComplete()));
  _config(this);
}

myPluginExecute::~myPluginExecute() {
  //delete m_base;
  delete m_IOController;
}

void myPluginExecute::refresh() {}

void myPluginExecute::setSetting(const QString& name, const QVariant& value) {
    ExecutePlugin::setSetting(name, value);
    if (name == QLatin1String("autoconfig")) {
        QStringList data = value.toString().split(QLatin1Char(':'));
	if (data.size()>=5)
	  m_IOController->connectToIOs(data[1].toInt(), data[2].toInt(), data[3], data[4]);
    }
}

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
void myPluginExecute::dataLoadingComplete() {
    emit pluginLoadingComplete(this);
}
