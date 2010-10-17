#include "plugin.h"
#include <QDebug>
#include "services/eventremotekey.h"
#include "statetracker/remotecontrolstatetracker.h"
#include "statetracker/remotecontrolkeystatetracker.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
	QString::fromAscii(EventRemoteKey::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
	QString::fromAscii(RemoteControlStateTracker::staticMetaObject.className()) <<
	QString::fromAscii(RemoteControlKeyStateTracker::staticMetaObject.className());
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == RemoteControlStateTracker::staticMetaObject.className()) {
        return new RemoteControlStateTracker();
    } else if (idb == RemoteControlKeyStateTracker::staticMetaObject.className()) {
        return new RemoteControlKeyStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == EventRemoteKey::staticMetaObject.className()) {
        return new EventRemoteKey();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
  qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Remote Control liri");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
