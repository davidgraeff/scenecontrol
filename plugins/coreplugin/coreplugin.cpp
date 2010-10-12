#include "coreplugin.h"
#include "services/systemAC.h"
#include "services/profileAC.h"
#include "services/backupAC.h"
#include "services/systemEV.h"
#include "statetracker/backupST.h"
#include "statetracker/systemST.h"
#include <shared/profile.h>
#include <shared/category.h>
#include "services/modeAC.h"
#include "statetracker/modeST.h"

QStringList CorePlugin::registerServices() const {
    return QStringList() <<
    QString::fromAscii(EventSystem::staticMetaObject.className()) <<
    QString::fromAscii(ActorSystem::staticMetaObject.className()) <<
    QString::fromAscii(ActorBackup::staticMetaObject.className()) <<
    QString::fromAscii(ActorCollection::staticMetaObject.className()) <<
    QString::fromAscii(ActorMode::staticMetaObject.className()) <<
    QString::fromAscii(Category::staticMetaObject.className()) <<
	QString::fromAscii(Collection::staticMetaObject.className());

}
QStringList CorePlugin::registerStateTracker() const {
    return QStringList() <<
    QString::fromAscii(SystemStateTracker::staticMetaObject.className()) <<
    QString::fromAscii(ModeStateTracker::staticMetaObject.className()) <<
    QString::fromAscii(BackupStateTracker::staticMetaObject.className());
}

AbstractStateTracker* CorePlugin::createStateTracker(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == SystemStateTracker::staticMetaObject.className()) {
        return new SystemStateTracker();
	} else if (idb == ModeStateTracker::staticMetaObject.className()) {
		return new ModeStateTracker();
	} else if (idb == BackupStateTracker::staticMetaObject.className()) {
		return new BackupStateTracker();
	}
    return 0;
}
AbstractServiceProvider* CorePlugin::createServiceProvider(const QString& id) {
    QByteArray idb = id.toAscii();
    if (idb == ActorSystem::staticMetaObject.className()) {
        return new ActorSystem();
    } else if (idb == ActorCollection::staticMetaObject.className()) {
		return new ActorCollection();
	} else if (idb == ActorMode::staticMetaObject.className()) {
		return new ActorMode();
	} else if (idb == ActorBackup::staticMetaObject.className()) {
		return new ActorBackup();
    } else if (idb == EventSystem::staticMetaObject.className()) {
        return new EventSystem();
	} else if (idb == Category::staticMetaObject.className()) {
		return new Category();
	} else if (idb == Collection::staticMetaObject.className()) {
		return new Collection();
	}
    return 0;
}

CorePlugin::CorePlugin() {
}
CorePlugin::~CorePlugin() {
}
QString CorePlugin::name() const {
    return QLatin1String("CorePlugin");
}
QString CorePlugin::version() const {
    return QLatin1String("1.0");
}
