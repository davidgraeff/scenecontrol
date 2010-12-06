#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "mediacontroller.h"
#include "services/actorpulsesink.h"
#include "statetracker/pulsestatetracker.h"
#include "services_server/actorpulsesinkServer.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
    m_base = new myPlugin();
    m_mediacontroller = new MediaController(this);
    connect(m_mediacontroller,SIGNAL(stateChanged(AbstractStateTracker*)),SIGNAL(stateChanged(AbstractStateTracker*)));
}

myPluginExecute::~myPluginExecute() {
    //delete m_base;
    delete m_mediacontroller;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorPulseSink::staticMetaObject.className())
        return new ActorPulseSinkServer((ActorPulseSink*)service, this);
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    return m_mediacontroller->getStateTracker();
}
MediaController* myPluginExecute::mediacontroller() {
    return m_mediacontroller;
}
