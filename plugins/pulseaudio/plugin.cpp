#include "plugin.h"
#include <QDebug>
#include "services/actorpulsesink.h"
#include "statetracker/pulsestatetracker.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
           QString::fromAscii(ActorPulseSink::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
           QString::fromAscii(PulseStateTracker::staticMetaObject.className());
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == PulseStateTracker::staticMetaObject.className())
        return new PulseStateTracker();
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorPulseSink::staticMetaObject.className())
        return new ActorPulseSink();
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
}
QString myPlugin::name() const {
    return QLatin1String("PulseAudio");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
