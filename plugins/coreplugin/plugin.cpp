#include "plugin.h"
#include "services/systemAC.h"
#include "services/profileAC.h"
#include "services/backupAC.h"
#include "services/systemEV.h"
#include "statetracker/backupST.h"
#include "statetracker/systemST.h"
#include "services/modeAC.h"
#include "statetracker/modeST.h"
#include "services/actorevent.h"
#include "services/actoreventvolume.h"
#include "statetracker/eventST.h"
#include "statetracker/eventvolumeST.h"
#include "services/modeCO.h"
#include "services/modeEV.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
           QString::fromAscii(EventSystem::staticMetaObject.className()) <<
           QString::fromAscii(ActorSystem::staticMetaObject.className()) <<
           QString::fromAscii(ActorBackup::staticMetaObject.className()) <<
           QString::fromAscii(ActorCollection::staticMetaObject.className()) <<
           QString::fromAscii(ActorMode::staticMetaObject.className()) <<
           QString::fromAscii(ConditionMode::staticMetaObject.className()) <<
           QString::fromAscii(EventMode::staticMetaObject.className()) <<
           QString::fromAscii(ActorEvent::staticMetaObject.className()) <<
           QString::fromAscii(ActorEventVolume::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
           QString::fromAscii(SystemStateTracker::staticMetaObject.className()) <<
           QString::fromAscii(ModeStateTracker::staticMetaObject.className()) <<
           QString::fromAscii(EventStateTracker::staticMetaObject.className()) <<
           QString::fromAscii(EventVolumeStateTracker::staticMetaObject.className()) <<
           QString::fromAscii(BackupStateTracker::staticMetaObject.className());
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == SystemStateTracker::staticMetaObject.className()) {
        return new SystemStateTracker();
    } else if (idb == ModeStateTracker::staticMetaObject.className()) {
        return new ModeStateTracker();
    } else if (idb == BackupStateTracker::staticMetaObject.className()) {
        return new BackupStateTracker();
    } else if (idb == EventStateTracker::staticMetaObject.className()) {
        return new EventStateTracker();
    } else if (idb == EventVolumeStateTracker::staticMetaObject.className()) {
        return new EventVolumeStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorSystem::staticMetaObject.className()) {
        return new ActorSystem();
    } else if (idb == ActorCollection::staticMetaObject.className()) {
        return new ActorCollection();
    } else if (idb == ActorMode::staticMetaObject.className()) {
        return new ActorMode();
    } else if (idb == ConditionMode::staticMetaObject.className()) {
        return new ConditionMode();
    } else if (idb == EventMode::staticMetaObject.className()) {
        return new EventMode();
    } else if (idb == ActorBackup::staticMetaObject.className()) {
        return new ActorBackup();
    } else if (idb == ActorEvent::staticMetaObject.className()) {
        return new ActorEvent();
    } else if (idb == ActorEventVolume::staticMetaObject.className()) {
        return new ActorEventVolume();
    } else if (idb == EventSystem::staticMetaObject.className()) {
        return new EventSystem();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
}
QString myPlugin::name() const {
    return QLatin1String("CorePlugin");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
