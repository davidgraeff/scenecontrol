#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "mediacontroller.h"
#include "services/conditionmusicstate.h"
#include "services/actorplaylistvolume.h"
#include "services/actorplaylisttrack.h"
#include "services/actorplaylistposition.h"
#include "services/actormusiccmd.h"
#include "statetracker/volumestatetracker.h"
#include "services_server/conditionmusicstateServer.h"
#include "services_server/actorplaylistvolumeServer.h"
#include "services_server/actorplaylisttrackServer.h"
#include "services_server/actorplaylistpositionServer.h"
#include "services_server/actorplaylistcmdServer.h"
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
    m_base = new myPlugin();
    m_mediacontroller = new MediaController(this);
    connect(m_mediacontroller,SIGNAL(stateChanged(AbstractStateTracker*)),SIGNAL(stateChanged(AbstractStateTracker*)));
    _config(this);
}

myPluginExecute::~myPluginExecute() {
    //delete m_base;
    delete m_mediacontroller;
}
bool ConditionMusicStateServer::checkcondition() {
    return (service<ConditionMusicState>()->value() == m_plugin->mediacontroller()->state());
}

void ActorPlaylistVolumeServer::execute() {
  ActorPlaylistVolume* base = service<ActorPlaylistVolume>();
    m_plugin->mediacontroller()->setVolume(base->volume(), base->relative());
}
void ActorPlaylistTrackServer::execute()
{
    MediaController* mc = m_plugin->mediacontroller();
    ActorPlaylistTrack* base = service<ActorPlaylistTrack>();
    // set playlist
    if (base->playlistid().size())
    {
        mc->setPlaylist(base->playlistid());
    }
    // set track number
    if (base->track() != -1)
    {
        mc->setCurrentTrack(base->track());
    }

    if (base->state() == MediaStateTracker::PlayState)
    {
        mc->play();
    } else if (base->state() == MediaStateTracker::PauseState)
    {
        mc->pause();
    } else if (base->state() == MediaStateTracker::StopState)
    {
        mc->stop();
    }
}
void ActorPlaylistPositionServer::execute() {
    ActorPlaylistPosition* base = service<ActorPlaylistPosition>();
    m_plugin->mediacontroller()->setTrackPosition(base->value(), base->relative());
}

void ActorPlaylistCmdServer::execute()
{
    ActorPlaylistCmd* base = service<ActorPlaylistCmd>();
    MediaController* mc = m_plugin->mediacontroller();
    if (base->cmd() == ActorPlaylistCmd::PlayCmd)
    {
        mc->play();
    } else if (base->cmd() == ActorPlaylistCmd::PauseCmd)
    {
        mc->pause();
    } else if (base->cmd() == ActorPlaylistCmd::StopCmd)
    {
        mc->stop();
    } else if (base->cmd() == ActorPlaylistCmd::NextCmd)
    {
        mc->next();
    } else if (base->cmd() == ActorPlaylistCmd::PrevCmd)
    {
        mc->previous();
    } else if (base->cmd() == ActorPlaylistCmd::NextPlaylistCmd)
    {
        mc->nextPlaylist();
    } else if (base->cmd() == ActorPlaylistCmd::PrevPlaylistCmd)
    {
        mc->previousPlaylist();
    } else if (base->cmd() == ActorPlaylistCmd::DumpMediaInfoCmd)
    {
        mc->dumpMediaInfo();
    }
}
void myPluginExecute::refresh() {}

void myPluginExecute::setSetting(const QString& name, const QVariant& value) {
    ExecutePlugin::setSetting(name, value);
    if (name == QLatin1String("server")) {
      m_mediacontroller->connectToMpd(value.toString());
    }
}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
	if (idb == ActorPlaylistCmd::staticMetaObject.className())
        return new ActorPlaylistCmdServer((ActorPlaylistCmd*)service, this);
    else if (idb == ActorPlaylistPosition::staticMetaObject.className())
        return new ActorPlaylistPositionServer((ActorPlaylistPosition*)service, this);
    else if (idb == ActorPlaylistTrack::staticMetaObject.className())
        return new ActorPlaylistTrackServer((ActorPlaylistTrack*)service, this);
    else if (idb == ActorPlaylistVolume::staticMetaObject.className())
        return new ActorPlaylistVolumeServer((ActorPlaylistVolume*)service, this);
    else if (idb == ConditionMusicState::staticMetaObject.className())
        return new ConditionMusicStateServer((ConditionMusicState*)service, this);
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    return m_mediacontroller->getStateTracker();
}
MediaController* myPluginExecute::mediacontroller() {
    return m_mediacontroller;
}
