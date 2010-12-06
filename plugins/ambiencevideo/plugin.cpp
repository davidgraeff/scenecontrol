#include "plugin.h"
#include <QDebug>
#include "services/actorambiencevideo.h"
#include "services/actorambiencevolume.h"
#include "services/actorambiencecmd.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
           QString::fromAscii(ActorAmbienceVideo::staticMetaObject.className()) <<
           QString::fromAscii(ActorAmbienceCmd::staticMetaObject.className()) <<
           QString::fromAscii(ActorAmbienceVolume::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList();
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
	Q_UNUSED(id);
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
	return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
    qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Ambience Video");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
