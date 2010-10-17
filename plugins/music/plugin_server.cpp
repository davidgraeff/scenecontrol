#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "../server/executeservice.h"
#include "plugin.h"
#include "mediacontroller.h"
#include "services/actormute.h"
#include "services/conditionmusicstate.h"
#include "services/actorplaylistvolume.h"
#include "services/actorplaylisttrack.h"
#include "services/actorplaylistposition.h"
#include "services/actorplaylistcmd.h"
#include "statetracker/pastatetracker.h"
#include "statetracker/volumestatetracker.h"
#include "services/playlist.h"
#include "services_server/playlistServer.h"
#include "services_server/conditionmusicstateServer.h"
#include "services_server/actorplaylistvolumeServer.h"
#include "services_server/actorplaylisttrackServer.h"
#include "services_server/actorplaylistpositionServer.h"
#include "services_server/actorplaylistcmdServer.h"
#include "services_server/actormuteServer.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
    m_base = new myPlugin();
    m_mediacontroller = new MediaController(this);
    connect(m_mediacontroller,SIGNAL(pluginobjectChanged(ExecuteWithBase*)),SIGNAL(pluginobjectChanged(ExecuteWithBase*)));
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
    if (idb == ActorMute::staticMetaObject.className())
        return new ActorMuteServer((ActorMute*)service, this);
    else if (idb == ActorPlaylistCmd::staticMetaObject.className())
        return new ActorPlaylistCmdServer((ActorPlaylistCmd*)service, this);
    else if (idb == ActorPlaylistPosition::staticMetaObject.className())
        return new ActorPlaylistPositionServer((ActorPlaylistPosition*)service, this);
    else if (idb == ActorPlaylistTrack::staticMetaObject.className())
        return new ActorPlaylistTrackServer((ActorPlaylistTrack*)service, this);
    else if (idb == ActorPlaylistVolume::staticMetaObject.className())
        return new ActorPlaylistVolumeServer((ActorPlaylistVolume*)service, this);
    else if (idb == ConditionMusicState::staticMetaObject.className())
        return new ConditionMusicStateServer((ConditionMusicState*)service, this);
    else if (idb == ActorPlaylist::staticMetaObject.className()) {
        ActorPlaylistServer* playlist =  new ActorPlaylistServer((ActorPlaylist*)service, this);
	m_mediacontroller->addedPlaylist(playlist);
        return playlist;
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    return m_mediacontroller->getStateTracker();
}
MediaController* myPluginExecute::mediacontroller() {
    return m_mediacontroller;
}
