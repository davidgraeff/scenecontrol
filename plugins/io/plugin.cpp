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

bool ConditionPinServer::checkcondition()
{
  return (m_plugin->controller()->getPin(service<ConditionPin>()->pin()) == service<ConditionPin>()->value());
}
void ActorPinServer::execute()
{
  ActorPin* a = service<ActorPin>();
  if (a->value()==ActorPin::PinToggle)
    m_plugin->controller()->togglePin(a->pin());
  else
    m_plugin->controller()->setPin(a->pin(),a->value());
}
void ActorPinNameServer::execute()
{
  ActorPinName* a = service<ActorPinName>();
  m_plugin->controller()->setPinName(a->pin(),a->pinname());
}
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
        if (data.size()>=4)
          m_IOController->connectToIOs(data[0].toInt(), data[1].toInt(), data[2], data[3]);
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
