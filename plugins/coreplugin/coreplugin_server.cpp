#include "coreplugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include "services/systemAC.h"
#include "services/profileAC.h"
#include "services/backupAC.h"
#include "services/systemEV.h"
#include "statetracker/systemST.h"
#include "statetracker/backupST.h"
#include "services_server/systemACServer.h"
#include "services_server/profileACServer.h"
#include "services_server/backupACServer.h"
#include "services_server/systemEVServer.h"
#include "shared/category.h"
#include "shared/profile.h"
#include "../server/executeprofile.h"
#include "../server/servicecontroller.h"
#include "../server/executeservice.h"
#include "statetracker/modeST.h"
#include "statetracker/eventST.h"
#include "statetracker/eventvolumeST.h"
#include <../server/eventcontroller.h>
#include "services/actorevent.h"
#include "services/actoreventvolume.h"
#include "services_server/actoreventServer.h"
#include "services_server/actoreventvolumeServer.h"

CorePluginExecute::CorePluginExecute( ServiceController* services, QObject* parent) : m_services(services) {
  Q_UNUSED(parent);
    m_base = new CorePlugin();
    m_BackupStateTracker = new BackupStateTracker();
    m_SystemStateTracker = new SystemStateTracker();
    m_ModeStateTracker = new ModeStateTracker();
    m_EventStateTracker = new EventStateTracker();
    m_EventVolumeStateTracker = new EventVolumeStateTracker();
    connect(m_services->eventcontroller(),SIGNAL(finished(QString,QString)),SLOT(finished(QString,QString)));
    connect(m_services->eventcontroller(),SIGNAL(started(QString,QString)),SLOT(started(QString,QString)));
    connect(m_services->eventcontroller(),SIGNAL(volumeChanged(qreal)),SLOT(volumeChanged(qreal)));
}

CorePluginExecute::~CorePluginExecute() {
    delete m_BackupStateTracker;
    delete m_SystemStateTracker;
    delete m_ModeStateTracker;
    delete m_EventStateTracker;
    delete m_EventVolumeStateTracker;
    delete m_base;
}

void CorePluginExecute::refresh() {}

ExecuteWithBase* CorePluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorSystem::staticMetaObject.className()) {
        return new ActorSystemServer((ActorSystem*)service, this);
    } else if (idb == ActorCollection::staticMetaObject.className()) {
        return new ActorCollectionServer((ActorCollection*)service, this);
    } else if (idb == ActorBackup::staticMetaObject.className()) {
        return new ActorBackupServer((ActorBackup*)service, this);
    } else if (idb == EventSystem::staticMetaObject.className()) {
        return new EventSystemServer((EventSystem*)service, this);
    } else if (idb == ActorEvent::staticMetaObject.className()) {
        return new ActorEventServer((ActorEvent*)service, this);
    } else if (idb == ActorEventVolume::staticMetaObject.className()) {
        return new ActorEventVolumeServer((ActorEventVolume*)service, this);
    } else if (idb == Category::staticMetaObject.className()) {
        return new ExecuteWithBase(service, this);
    } else if (idb == Collection::staticMetaObject.className()) {
        return new ExecuteCollection(service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> CorePluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    m_SystemStateTracker->setApp(QCoreApplication::applicationVersion());
    m_SystemStateTracker->setMin(QCoreApplication::applicationVersion());
    m_SystemStateTracker->setMax(QCoreApplication::applicationVersion());
    m_BackupStateTracker->setBackups(backups());
    m_ModeStateTracker->setMode(m_services->mode());
    m_EventStateTracker->setFilename(m_services->eventcontroller()->filename());
    m_EventStateTracker->setTitle(m_services->eventcontroller()->title());
    m_EventStateTracker->setState(m_services->eventcontroller()->state());
    m_EventVolumeStateTracker->setVolume(m_services->eventcontroller()->volume()*100);
    a.append(m_SystemStateTracker);
    a.append(m_BackupStateTracker);
    a.append(m_ModeStateTracker);
    a.append(m_EventStateTracker);
    a.append(m_EventVolumeStateTracker);
    return a;
}

void CorePluginExecute::backup()
{
    QDir destdir = m_services->saveDir().filePath ( QDateTime::currentDateTime().toString() );
    if ( !destdir.exists() && !destdir.mkpath ( destdir.absolutePath() ) )
    {
        qDebug() << "Backup failed" << destdir;
        return;
    }
    QDir sourcedir = m_services->saveDir();
    QStringList files = sourcedir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    qDebug() << "Backup" << files.size() << "files to" << destdir.path();
    foreach ( QString file, files )
    {
        QFile::remove ( destdir.absoluteFilePath ( file ) );
        if ( !QFile::copy ( sourcedir.absoluteFilePath ( file ), destdir.absoluteFilePath ( file ) ) )
        {
            qDebug() << "Backup of"<<file<<"failed";
        }
    }

    m_BackupStateTracker->setBackups(backups());
    emit stateChanged(m_BackupStateTracker);
}

void CorePluginExecute::backup_remove ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = m_services->saveDir();
    if ( !destdir.cd ( id ) ) return;
    if ( destdir.exists() )
    {
        QStringList files = destdir.entryList ( QDir::Files );
        foreach ( QString file, files ) destdir.remove ( file );
        destdir.rmdir ( destdir.absolutePath() );
    }

    m_BackupStateTracker->setBackups(backups());
    emit stateChanged(m_BackupStateTracker);
}

void CorePluginExecute::backup_restore ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir sourcedir = m_services->saveDir();
    if ( !sourcedir.cd ( id ) ) return;

    //remove current
    QStringList files = m_services->saveDir().entryList ( QDir::Files );
    foreach ( QString file, files ) m_services->saveDir().remove ( file );

    //restore backup
    files = sourcedir.entryList ( QDir::Files );
    foreach ( QString file, files )
    QFile::copy ( sourcedir.filePath ( file ),m_services->saveDir().filePath ( file ) );
}

QStringList CorePluginExecute::backups()
{
    return m_services->saveDir().entryList ( QDir::Dirs|QDir::NoDotAndDotDot );
}
ServiceController* CorePluginExecute::serviceController() {
    return m_services;
}
void CorePluginExecute::setMode(const QString& mode) {
    m_services->setMode(mode);
    m_ModeStateTracker->setMode(mode);
    emit stateChanged(m_ModeStateTracker);
    emit modeChanged();
}
void CorePluginExecute::started(const QString& eventTitle, const QString& filename) {
    m_EventStateTracker->setFilename(filename);
    m_EventStateTracker->setTitle(eventTitle);
    m_EventStateTracker->setState(1);
    emit stateChanged(m_EventStateTracker);
}
void CorePluginExecute::finished(const QString& eventTitle, const QString& filename) {
    m_EventStateTracker->setFilename(filename);
    m_EventStateTracker->setTitle(eventTitle);
    m_EventStateTracker->setState(0);
    emit stateChanged(m_EventStateTracker);
}
void CorePluginExecute::volumeChanged(qreal vol) {
    m_EventVolumeStateTracker->setVolume(m_services->eventcontroller()->volume()*100);
    emit stateChanged(m_EventVolumeStateTracker);
}

