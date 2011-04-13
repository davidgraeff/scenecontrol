#include "backups.h"
#include <QDateTime>
#include <QDebug>
#include "paths.h"

Backups::Backups()
{
} 

Backups::~Backups()
{

}

QList< QVariantMap > Backups::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
	l.append(ServiceCreation::createModelReset(PLUGIN_ID, "backup").getData());
	QDir backupdir = serviceBackupDir();
    QStringList backups = backupdir.entryList ( QDir::Dirs|QDir::NoDotAndDotDot );
    foreach(QString dir, backups) {
		ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "backup");
        QFile namefile(backupdir.absoluteFilePath ( dir ));
        namefile.open(QIODevice::ReadOnly|QIODevice::Truncate);
		sc.setData("backupid", dir);
		sc.setData("name", QString::fromUtf8(namefile.readLine()));
        namefile.close();
		l.append(sc.getData());
    }
    return l;
}

bool Backups::condition(const QVariantMap& data)  {
    Q_UNUSED(data);
	return false;
}

void Backups::event_changed(const QVariantMap& data)  {
    Q_UNUSED(data);
}

void Backups::execute(const QVariantMap& data)  {
    if (ServiceID::isId(data,"backup_create")) {
		create(DATA("name"));
	} else if (ServiceID::isId(data,"backup_restore")) {
		restore(DATA("backupid"));
	} else if (ServiceID::isId(data,"backup_remove")) {
		remove(DATA("backupid"));
    }
}

void Backups::initialize() {
}

void Backups::clear() {
}

void Backups::create(const QString& name)
{
	const QString id = QDateTime::currentDateTime().toString();
    QDir destdir = serviceBackupDir().filePath ( id );
    if ( !destdir.exists() && !destdir.mkpath ( destdir.absolutePath() ) )
    {
        qWarning() << "Backup failed" << destdir;
        return;
    }
    QDir sourcedir = serviceBackupDir();
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
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "backup");
	sc.setData("backupid", id);
	sc.setData("name", name);
	m_server->property_changed(sc.getData());
}

void Backups::rename(const QString& id, const QString& name)
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = serviceBackupDir();
    if ( !destdir.cd ( id ) ) return;
    if ( destdir.exists() )
    {
        QFile namefile(destdir.absoluteFilePath ( QLatin1String("name.txt") ));
        namefile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        namefile.write(name.toUtf8());
        namefile.close();
    }
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "backup");
	sc.setData("backupid", id);
	sc.setData("name", name);
	m_server->property_changed(sc.getData());
}

void Backups::remove ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = serviceBackupDir();
    if ( !destdir.cd ( id ) ) return;
    if ( destdir.exists() )
    {
        QStringList files = destdir.entryList ( QDir::Files );
        foreach ( QString file, files ) destdir.remove ( file );
        destdir.rmdir ( destdir.absolutePath() );
    }
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "backup");
	sc.setData("backupid", id);
	sc.setData("name", QString());
	m_server->property_changed(sc.getData());
}

void Backups::restore ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
	QDir destdir = serviceBackupDir();
    QDir sourcedir = serviceBackupDir();
    if ( !sourcedir.cd ( id ) ) return;

    //remove current
    QStringList files = destdir.entryList ( QDir::Files );
    foreach ( QString file, files ) destdir.remove ( file );

    //restore backup
    files = sourcedir.entryList ( QDir::Files );
    foreach ( QString file, files ) {
		QFile::copy ( sourcedir.filePath ( file ),destdir.filePath ( file ) );
	}
}

