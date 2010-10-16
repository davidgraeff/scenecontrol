#include "plugin.h"
#include <QDebug>
#include "services/actorprojector.h"

QStringList myPlugin::registerServices() const {
    return QStringList() <<
	QString::fromAscii(ActorProjector::staticMetaObject.className());

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
    if (idb == ActorProjector::staticMetaObject.className()) {
        return new ActorProjector();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
  qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Wake up on lan");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
