#include "backups.h"
#include <QDateTime>
#include <QDebug>

Backups::Backups()
{
	m_savedir = QDir::home();
} 

Backups::~Backups()
{

}

QMap<QString, QVariantMap> Backups::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QMap<QString, QVariantMap> l;
    QStringList backups = m_savedir.entryList ( QDir::Dirs|QDir::NoDotAndDotDot );
    foreach(QString dir, backups) {
		PROPERTY("backup");
        QFile namefile(m_savedir.absoluteFilePath ( dir ));
        namefile.open(QIODevice::ReadOnly|QIODevice::Truncate);
		data[QLatin1String("backupid")] = dir;
		data[QLatin1String("name")] = QString::fromUtf8(namefile.readLine());
        namefile.close();
		l.insertMulti(QLatin1String("backup"), data);
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
    if (IS_ID("backup_create")) {
		create(DATA("name"));
	} else if (IS_ID("backup_restore")) {
		restore(DATA("backupid"));
	} else if (IS_ID("backup_remove")) {
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
    QDir destdir = m_savedir.filePath ( id );
    if ( !destdir.exists() && !destdir.mkpath ( destdir.absolutePath() ) )
    {
        qWarning() << "Backup failed" << destdir;
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
    PROPERTY("backup");
	data[QLatin1String("backupid")] = id;
	data[QLatin1String("name")] = name;
	m_server->property_changed(data);
}

void Backups::rename(const QString& id, const QString& name)
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
    }
    PROPERTY("backup");
	data[QLatin1String("backupid")] = id;
	data[QLatin1String("name")] = name;
	m_server->property_changed(data);
}

void Backups::remove ( const QString& id )
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
    PROPERTY("backup");
	data[QLatin1String("backupid")] = id;
	data[QLatin1String("name")] = QString();
	m_server->property_changed(data);
}

void Backups::restore ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir sourcedir = m_savedir;
    if ( !sourcedir.cd ( id ) ) return;

    //remove current
    QStringList files = m_savedir.entryList ( QDir::Files );
    foreach ( QString file, files ) m_savedir.remove ( file );

    //restore backup
    files = sourcedir.entryList ( QDir::Files );
    foreach ( QString file, files ) {
		QFile::copy ( sourcedir.filePath ( file ),m_savedir.filePath ( file ) );
	}
}

