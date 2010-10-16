#include "plugin.h"
#include <QDebug>
#include "services/eventperiodic.h"
#include "services/eventdatetime.h"
#include "services/conditiontimespan.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
           QString::fromAscii(EventPeriodic::staticMetaObject.className())<<
           QString::fromAscii(EventDateTime::staticMetaObject.className())<<
           QString::fromAscii(ConditionTimespan::staticMetaObject.className());

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
    if (idb == EventPeriodic::staticMetaObject.className()) {
        return new EventPeriodic();
    } else if (idb == EventDateTime::staticMetaObject.className()) {
        return new EventDateTime();
    } else if (idb == ConditionTimespan::staticMetaObject.className()) {
        return new ConditionTimespan();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
    qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Time Related");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
