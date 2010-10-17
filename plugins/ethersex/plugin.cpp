#include "plugin.h"
#include <QDebug>
#include "services/actorcurtain.h"
#include "services/conditionled.h"
#include "services/conditioncurtain.h"
#include "services/actorledname.h"
#include "services/actorled.h"
#include "statetracker/curtainstatetracker.h"
#include "statetracker/lednamestatetracker.h"
#include "statetracker/ledvaluestatetracker.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
	QString::fromAscii(ActorCurtain::staticMetaObject.className())<<
	QString::fromAscii(ActorLed::staticMetaObject.className())<<
	QString::fromAscii(ActorLedName::staticMetaObject.className())<<
	QString::fromAscii(ConditionCurtain::staticMetaObject.className())<<
	QString::fromAscii(ConditionLed::staticMetaObject.className());

}
QStringList myPlugin::registerStateTracker() const {
    return QStringList() <<
	QString::fromAscii(CurtainStateTracker::staticMetaObject.className())<<
	QString::fromAscii(ChannelNameStateTracker::staticMetaObject.className())<<
	QString::fromAscii(ChannelValueStateTracker::staticMetaObject.className());
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == CurtainStateTracker::staticMetaObject.className()) {
        return new CurtainStateTracker();
    } else if (idb == ChannelNameStateTracker::staticMetaObject.className()) {
        return new ChannelNameStateTracker();
    } else if (idb == ChannelValueStateTracker::staticMetaObject.className()) {
        return new ChannelValueStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorCurtain::staticMetaObject.className()) {
        return new ActorCurtain();
    } else if (idb == ActorLed::staticMetaObject.className()) {
        return new ActorLed();
    } else if (idb == ActorLedName::staticMetaObject.className()) {
        return new ActorLedName();
    } else if (idb == ConditionCurtain::staticMetaObject.className()) {
        return new ConditionCurtain();
    } else if (idb == ConditionLed::staticMetaObject.className()) {
        return new ConditionLed();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
  qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Led and curtain");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
