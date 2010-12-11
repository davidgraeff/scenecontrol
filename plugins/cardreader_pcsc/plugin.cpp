#include "plugin.h"
#include "services/eventcardreader.h"
#include "statetracker/cardreaderpcscstatetracker.h"

#include <QDebug>

QStringList myPlugin::registerServices() const {
    return QStringList() <<
	QString::fromAscii(EventCardReader::staticMetaObject.className());
}

QStringList myPlugin::registerStateTracker() const {
	return QStringList()<<
	QString::fromAscii(CardReaderPCSCStateTracker::staticMetaObject.className());
}

AbstractStateTracker* myPlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == CardReaderPCSCStateTracker::staticMetaObject.className()) {
        return new CardReaderPCSCStateTracker();
    }
    return 0;
}
AbstractServiceProvider* myPlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == EventCardReader::staticMetaObject.className()) {
        return new EventCardReader();
    }
    return 0;
}

myPlugin::myPlugin() {
}
myPlugin::~myPlugin() {
  qDebug() <<"free";
}
QString myPlugin::name() const {
    return QLatin1String("Cardreader PCSC");
}
QString myPlugin::version() const {
    return QLatin1String("1.0");
}
