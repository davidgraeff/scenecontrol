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

void Backups::create(const QString& name)
{
    QDir destdir = m_savedir.filePath ( QDateTime::currentDateTime().toString() );
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

QStringList Backups::list()
{
    QStringList backups = m_savedir.entryList ( QDir::Dirs|QDir::NoDotAndDotDot );
    QStringList names;
    foreach(QString dir, backups) {
        QFile namefile(m_savedir.absoluteFilePath ( dir ));
        namefile.open(QIODevice::ReadOnly|QIODevice::Truncate);
        names.append(QString::fromUtf8(namefile.readLine()));
        namefile.close();
    }
    return backups;
}
