#include "plugin.h"
#include <QDebug>
#include "services/actorpin.h"
#include "services/actorpinname.h"
#include "services/conditionpin.h"
#include "statetracker/pinnamestatetracker.h"
#include "statetracker/pinvaluestatetracker.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
	QString::fromAscii(ActorPin::staticMetaObject.className())<<
	QString::fromAscii(ActorPinName::staticMetaObject.className())<<
	QString::fromAscii(ConditionPin::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
	QString::fromAscii(PinNameStateTracker::staticMetaObject.className())<<
	QString::fromAscii(PinValueStateTracker::staticMetaObject.className());

}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == PinNameStateTracker::staticMetaObject.className()) {
        return new PinNameStateTracker();
    } else if (idb == PinValueStateTracker::staticMetaObject.className()) {
        return new PinValueStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorPin::staticMetaObject.className()) {
        return new ActorPin();
    } else if (idb == ActorPinName::staticMetaObject.className()) {
        return new ActorPinName();
    } else if (idb == ConditionPin::staticMetaObject.className()) {
        return new ConditionPin();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
  qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Steckdosen");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
