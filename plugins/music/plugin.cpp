#include "plugin.h"
#include <QDebug>
#include "services/actormute.h"
#include "services/actorplaylistcmd.h"
#include "services/actorplaylistposition.h"
#include "services/actorplaylisttrack.h"
#include "services/actorplaylistvolume.h"
#include "services/conditionmusicstate.h"
#include "statetracker/mediastatetracker.h"
#include "statetracker/pastatetracker.h"
#include "statetracker/volumestatetracker.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
           QString::fromAscii(ActorMute::staticMetaObject.className()) <<
           QString::fromAscii(ActorPlaylistCmd::staticMetaObject.className()) <<
           QString::fromAscii(ActorPlaylistPosition::staticMetaObject.className()) <<
           QString::fromAscii(ActorPlaylistTrack::staticMetaObject.className()) <<
           QString::fromAscii(ActorPlaylistVolume::staticMetaObject.className()) <<
           QString::fromAscii(ConditionMusicState::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
           QString::fromAscii(MediaStateTracker::staticMetaObject.className()) <<
           QString::fromAscii(PAStateTracker::staticMetaObject.className()) <<
           QString::fromAscii(MusicVolumeStateTracker::staticMetaObject.className());
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == MediaStateTracker::staticMetaObject.className())
        return new MediaStateTracker();
    else if (idb == PAStateTracker::staticMetaObject.className())
        return new PAStateTracker();
    else if (idb == MusicVolumeStateTracker::staticMetaObject.className())
        return new MusicVolumeStateTracker();
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorMute::staticMetaObject.className())
        return new ActorMute();
    else if (idb == ActorPlaylistCmd::staticMetaObject.className())
        return new ActorPlaylistCmd();
    else if (idb == ActorPlaylistPosition::staticMetaObject.className())
        return new ActorPlaylistPosition();
    else if (idb == ActorPlaylistTrack::staticMetaObject.className())
        return new ActorPlaylistTrack();
    else if (idb == ActorPlaylistVolume::staticMetaObject.className())
        return new ActorPlaylistVolume();
    else if (idb == ConditionMusicState::staticMetaObject.className())
        return new ConditionMusicState();
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
    qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Music");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
