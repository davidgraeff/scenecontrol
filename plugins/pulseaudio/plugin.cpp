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

void ActorPulseSinkServer::execute()
{
    ActorPulseSink* base = service<ActorPulseSink>();
    if (base->mute()==ActorPulseSink::MuteSink)
        m_plugin->mediacontroller()->setPAMute(base->sinkid().toAscii(),1);
    else if (base->mute()==ActorPulseSink::UnmuteSink)
        m_plugin->mediacontroller()->setPAMute(base->sinkid().toAscii(),0);
    else if (base->mute()==ActorPulseSink::ToggleSink)
        m_plugin->mediacontroller()->togglePAMute(base->sinkid().toAscii());

    if (base->assignment()==ActorPulseSink::NoVolumeSet) return;
    m_plugin->mediacontroller()->setPAVolume(base->sinkid().toAscii(),base->volume(),(base->assignment()==ActorPulseSink::VolumeRelative));
}
