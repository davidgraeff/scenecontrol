#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include "shared/server/executeservice.h"
#include "services/systemAC.h"
#include "services/profileAC.h"
#include "services/backupAC.h"
#include "services/systemEV.h"
#include <services/modeAC.h>
#include <services/modeCO.h>
#include <services/modeEV.h>
#include "statetracker/systemST.h"
#include "statetracker/backupST.h"
#include "statetracker/modeST.h"
#include "services_server/systemACServer.h"
#include "services_server/profileACServer.h"
#include "services_server/backupACServer.h"
#include "services_server/systemEVServer.h"
#include <services_server/modeACServer.h>
#include <services_server/modeCOServer.h>
#include <services_server/modeEVServer.h>
#include <QApplication>
#include <QtPlugin>

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

void ActorBackupServer::execute() {
	switch (m_base->action()) {
		case ActorBackup::CreateBackup:
			m_plugin->backup_create(m_base->backupname());
			break;
		case ActorBackup::RemoveBackup:
			m_plugin->backup_remove(m_base->backupid());
			break;
		case ActorBackup::RestoreBackup:
			m_plugin->backup_restore(m_base->backupid());
			break;
		case ActorBackup::RenameBackup:
			m_plugin->backup_rename(m_base->backupid(), m_base->backupname());
			break;
		default:
			break;
	};
}

void ActorModeServer::execute() {
	m_plugin->setMode(m_base->mode());
}

bool ConditionModeServer::checkcondition() {
	return (m_base->mode()==m_plugin->mode());
}

void EventSystemServer::systemStarted() {
	if (m_base->system()==EventSystem::ServerStarted) {
		emit trigger();
	}	
}

void EventModeServer::modeChanged() {
	if (m_base->mode().isEmpty() || m_base->mode()==m_plugin->mode()) {
		emit trigger();
	}
	
}


myPluginExecute::myPluginExecute( QObject* parent) {
    Q_UNUSED(parent);
    m_base = new myPlugin();
    m_ModeStateTracker = new ModeStateTracker();
    m_ModeStateTracker->setMode(m_mode);
    m_BackupStateTracker = new BackupStateTracker();
    m_SystemStateTracker = new SystemStateTracker();
    m_SystemStateTracker->setApp(QCoreApplication::applicationVersion());
    m_SystemStateTracker->setMin(QCoreApplication::applicationVersion());
    m_SystemStateTracker->setMax(QCoreApplication::applicationVersion());
    m_savedir = QDir(qApp->property("settingspath").value<QString>());
    backups_changed();
}

myPluginExecute::~myPluginExecute() {
    delete m_BackupStateTracker;
    delete m_SystemStateTracker;
    delete m_ModeStateTracker;
    delete m_base;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorSystem::staticMetaObject.className()) {
        return new ActorSystemServer((ActorSystem*)service, this);
    } else if (idb == ActorCollection::staticMetaObject.className()) {
		ActorCollectionServer* a = new ActorCollectionServer((ActorCollection*)service, this);
		connect(this,SIGNAL(_serviceChanged(AbstractServiceProvider*)),a,SLOT(serviceChanged(AbstractServiceProvider*)));
        return a;
    } else if (idb == ActorBackup::staticMetaObject.className()) {
        return new ActorBackupServer((ActorBackup*)service, this);
    } else if (idb == EventSystem::staticMetaObject.className()) {
        return new EventSystemServer((EventSystem*)service, this);
    } else if (idb == ActorMode::staticMetaObject.className()) {
        return new ActorModeServer((ActorMode*)service, this);
    } else if (idb == ConditionMode::staticMetaObject.className()) {
        return new ConditionModeServer((ConditionMode*)service, this);
    } else if (idb == EventMode::staticMetaObject.className()) {
        return new EventModeServer((EventMode*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    a.append(m_SystemStateTracker);
    a.append(m_BackupStateTracker);
    a.append(m_ModeStateTracker);
    return a;
}

void myPluginExecute::backup_create(const QString& name)
{
    QDir destdir = m_savedir.filePath ( QDateTime::currentDateTime().toString() );
    if ( !destdir.exists() && !destdir.mkpath ( destdir.absolutePath() ) )
    {
        qDebug() << "Backup failed" << destdir;
        return;
    }
    QDir sourcedir = m_savedir;
    QStringList files = sourcedir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    files.removeAll(QLatin1String("name.txt"));
    qDebug() << "Backup" << files.size() << "files to" << destdir.path();
    foreach ( QString file, files )
    {
        QFile::remove ( destdir.absoluteFilePath ( file ) );
        if ( !QFile::copy ( sourcedir.absoluteFilePath ( file ), destdir.absoluteFilePath ( file ) ) )
        {
            qDebug() << "Backup of"<<file<<"failed";
        }
    }
    QFile namefile(destdir.absoluteFilePath ( QLatin1String("name.txt") ));
    namefile.open(QIODevice::WriteOnly|QIODevice::Truncate);
    namefile.write(name.toUtf8());
    namefile.close();

    backups_changed();
    emit stateChanged(m_BackupStateTracker);
}

void myPluginExecute::backup_rename(const QString& id, const QString& name)
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = m_savedir;
    if ( !destdir.cd ( id ) ) return;
    if ( destdir.exists() )
    {
        QFile namefile(destdir.absoluteFilePath ( QLatin1String("name.txt") ));
        namefile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        namefile.write(name.toUtf8());
        namefile.close();
        backups_changed();
        emit stateChanged(m_BackupStateTracker);
    }
}

void myPluginExecute::backup_remove ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = m_savedir;
    if ( !destdir.cd ( id ) ) return;
    if ( destdir.exists() )
    {
        QStringList files = destdir.entryList ( QDir::Files );
        foreach ( QString file, files ) destdir.remove ( file );
        destdir.rmdir ( destdir.absolutePath() );
    }

    backups_changed();
    emit stateChanged(m_BackupStateTracker);
}

void myPluginExecute::backup_restore ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir sourcedir = m_savedir;
    if ( !sourcedir.cd ( id ) ) return;

    //remove current
    QStringList files = m_savedir.entryList ( QDir::Files );
    foreach ( QString file, files ) m_savedir.remove ( file );

    //restore backup
    files = sourcedir.entryList ( QDir::Files );
    foreach ( QString file, files )
    QFile::copy ( sourcedir.filePath ( file ),m_savedir.filePath ( file ) );
}

void myPluginExecute::backups_changed()
{
    QStringList backups = m_savedir.entryList ( QDir::Dirs|QDir::NoDotAndDotDot );
    m_BackupStateTracker->setBackupids(backups);
    QStringList names;
    foreach(QString dir, backups) {
        QFile namefile(m_savedir.absoluteFilePath ( dir ));
        namefile.open(QIODevice::ReadOnly|QIODevice::Truncate);
        names.append(QString::fromUtf8(namefile.readLine()));
        namefile.close();
    }
    m_BackupStateTracker->setBackupnames(names);
}

void myPluginExecute::setMode(const QString& mode) {
    m_mode = mode;
    m_ModeStateTracker->setMode(mode);
    emit stateChanged(m_ModeStateTracker);
}
void myPluginExecute::dataLoadingComplete() {
	emit pluginLoadingComplete(this);
}
