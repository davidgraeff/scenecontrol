#include "plugin.h"
#include <QDebug>
#include "services/actorambiencevideo.h"
#include "services/actorambiencevolume.h"
#include "services/actorambiencecmd.h"
#include "services/actorevent.h"
#include "services/actoreventvolume.h"
#include "statetracker/eventST.h"
#include "statetracker/eventvolumeST.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
           QString::fromAscii(ActorAmbienceVideo::staticMetaObject.className()) <<
           QString::fromAscii(ActorAmbienceCmd::staticMetaObject.className()) <<
           QString::fromAscii(ActorAmbienceVolume::staticMetaObject.className()) <<
           QString::fromAscii(ActorEvent::staticMetaObject.className()) <<
           QString::fromAscii(ActorEventVolume::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList();
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == EventStateTracker::staticMetaObject.className()) {
        return new EventStateTracker();
    } else if (idb == EventVolumeStateTracker::staticMetaObject.className()) {
        return new EventVolumeStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
	if (idb == ActorAmbienceVideo::staticMetaObject.className())
		return new ActorAmbienceVideo();
	else if (idb == ActorAmbienceVolume::staticMetaObject.className())
		return new ActorAmbienceVolume();
	else if (idb == ActorAmbienceCmd::staticMetaObject.className())
		return new ActorAmbienceCmd();
	else if (idb == ActorEvent::staticMetaObject.className()) {
        return new ActorEvent();
    } else if (idb == ActorEventVolume::staticMetaObject.className()) {
        return new ActorEventVolume();
    }
return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
}
QString myPlugin::name() const {
    return QLatin1String("Ambience Video");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
