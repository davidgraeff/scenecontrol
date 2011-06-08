#include "backups.h"
#include <QDateTime>
#include <QDebug>
#include "paths.h"

Backups::Backups()
{
    QDir backupdir = serviceBackupDir();
    QStringList backups = backupdir.entryList ( QDir::Dirs|QDir::NoDotAndDotDot );
    foreach(QString dir, backups) {
        Backup b;
        QDir backupdir_sub(backupdir);
        backupdir_sub.cd(dir);
        QFile namefile(backupdir_sub.absoluteFilePath ( QLatin1String("name.txt") ));
        namefile.open(QIODevice::ReadWrite);
        b.id = dir;
        b.files = backupdir_sub.entryList ( QDir::Files|QDir::NoDotAndDotDot ).size();
        if (namefile.canReadLine())
            b.name = QString::fromUtf8(namefile.readLine());
        if (namefile.canReadLine())
            b.date = QString::fromUtf8(namefile.readLine());
		if (b.name.isEmpty()) {
			b.name = dir;
			namefile.write(b.name.toUtf8());
		}
		if (b.date.isEmpty()) {
			b.date = QDateTime::currentDateTime().toString();
			namefile.write(b.date.toAscii());
		}
        namefile.close();
        m_backups[b.id] = b;
    }
}

Backups::~Backups()
{

}

QList< QVariantMap > Backups::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "backup", "backupid").getData());
    foreach(Backup b, m_backups) {
        l.append(changeNotify(b));
    }
    return l;
}

bool Backups::condition(const QVariantMap& data, const QString& sessionid)  {
    Q_UNUSED(data);
    Q_UNUSED(sessionid);
    return false;
}

void Backups::register_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(data);
    Q_UNUSED(collectionuid);
}

void Backups::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(data);
    Q_UNUSED(collectionuid);
}

void Backups::execute(const QVariantMap& data, const QString& sessionid)  {
    Q_UNUSED(sessionid);
    if (ServiceID::isId(data,"backup_create")) {
        create(DATA("name"));
    } else if (ServiceID::isId(data,"backup_restore")) {
        restore(DATA("backupid"));
    } else if (ServiceID::isId(data,"backup_remove")) {
        remove(DATA("backupid"));
    } else if (ServiceID::isId(data,"backup_rename")) {
        rename(DATA("backupid"), DATA("name"));
    }
}

void Backups::initialize() {
}

void Backups::clear() {
}

void Backups::create(const QString& name)
{
    const QString id = QDateTime::currentDateTime().toString(QLatin1String("yyyy_MM_dd_hh_mm_ss"));
    const QString date = QLocale(QLocale::English).toString(QDateTime::currentDateTime(), QLatin1String("ddd, d MMMM yyyy hh:mm:ss"));

    QDir destdir = serviceBackupDir().filePath ( id );
    if ( !destdir.exists() && !destdir.mkpath ( destdir.absolutePath() ) )
    {
        qWarning() << "Backup failed" << destdir;
        return;
    }
    QDir sourcedir = serviceDir();
    QStringList files = sourcedir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    files.removeAll(QLatin1String("name.txt"));
    qDebug() << "Backup" << files.size() << "files from"<< sourcedir.path() <<"to" << destdir.path();
    foreach ( QString file, files )
    {
        QFile::remove ( destdir.absoluteFilePath ( file ) );
        if ( !QFile::copy ( sourcedir.absoluteFilePath ( file ), destdir.absoluteFilePath ( file ) ) )
        {
            qDebug() << "Backup of"<<file<<"failed";
        }
    }
    Backup b;
    b.id = id;
    b.files = files.size();
    b.name = name;
    b.date = date;
    m_backups[b.id] = b;
    m_server->property_changed(changeNotify(b, true));
}

void Backups::rename(const QString& id, const QString& name)
{
    QDir destdir = serviceBackupDir().filePath ( id );
    if ( !m_backups.contains(id) || !destdir.exists() ) return;
    m_backups[id].name = name;
    m_server->property_changed(changeNotify(m_backups[id], true));
}

void Backups::remove ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = serviceBackupDir().filePath ( id );
    if ( destdir.exists() )
    {
        QStringList files = destdir.entryList ( QDir::Files );
        foreach ( QString file, files ) destdir.remove ( file );
        destdir.rmdir ( destdir.absolutePath() );
        ServiceCreation sc = ServiceCreation::createModelRemoveItem(PLUGIN_ID, "backup");
        sc.setData("backupid", id);
        m_server->property_changed(sc.getData());
    }
}

void Backups::restore ( const QString& id )
{
    if ( id.trimmed().isEmpty() ) return;
    QDir destdir = serviceBackupDir();
    QDir sourcedir = serviceBackupDir().filePath ( id );
    if ( !destdir.exists() ) return;

    //remove current
    QStringList files = destdir.entryList ( QDir::Files );
    foreach ( QString file, files ) destdir.remove ( file );

    //restore backup
    files = sourcedir.entryList ( QDir::Files );
    foreach ( QString file, files ) {
        QFile::copy ( sourcedir.filePath ( file ),destdir.filePath ( file ) );
    }
    qDebug() << "Backup restore " << files.size() << "files to" << destdir.path();
}

QVariantMap Backups::changeNotify(const Backups::Backup& b, bool write) {
    if (write) {
        QDir dir = serviceBackupDir().filePath ( b.id );
        QFile namefile(dir.absoluteFilePath ( QLatin1String("name.txt") ));
        namefile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        namefile.write(b.name.toUtf8()+'\n');
        namefile.write(b.date.toAscii());
        namefile.close();
    }

    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "backup");
    sc.setData("backupid", b.id);
    sc.setData("files", b.files);
    if (b.name.size())
        sc.setData("name", b.name);
    if (b.date.size())
        sc.setData("date", b.date);
    return sc.getData();
}

