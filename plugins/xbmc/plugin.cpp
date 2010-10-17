#include "plugin.h"
#include <QDebug>
#include "services/actorcinema.h"
#include "services/actorcinemavolume.h"
#include "services/actorcinemaposition.h"
#include "statetracker/cinamestatetracker.h"
#include "statetracker/volumestatetracker.h"
#include "statetracker/cinemapositionstatetracker.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
	QString::fromAscii(ActorCinema::staticMetaObject.className()) <<
	QString::fromAscii(ActorCinemaVolume::staticMetaObject.className()) <<
	QString::fromAscii(ActorCinemaPosition::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
	QString::fromAscii(CinemaStateTracker::staticMetaObject.className()) <<
	QString::fromAscii(CinemaVolumeStateTracker::staticMetaObject.className()) <<
	QString::fromAscii(CinemaPositionStateTracker::staticMetaObject.className());

}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == CinemaStateTracker::staticMetaObject.className()) {
        return new CinemaStateTracker();
    } else if (idb == CinemaVolumeStateTracker::staticMetaObject.className()) {
        return new CinemaVolumeStateTracker();
    } else if (idb == CinemaPositionStateTracker::staticMetaObject.className()) {
        return new CinemaPositionStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorCinema::staticMetaObject.className()) {
        return new ActorCinema();
    } else if (idb == ActorCinemaPosition::staticMetaObject.className()) {
        return new ActorCinemaPosition();
    } else if (idb == ActorCinemaVolume::staticMetaObject.className()) {
        return new ActorCinemaVolume();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
  qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Control xbmc");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
