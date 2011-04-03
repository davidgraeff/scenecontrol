#include "paths.h"

#include <QSettings>
#include "config.h"

#ifdef _WIN32
	const QString defaultpath = QLatin1String("");
#else
	const QString defaultpath = QLatin1String("/usr/");
#endif
	
QDir pluginDir() {
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_LIBPATH));
	return dir;
}

QString certificateFile(const QString& file) {
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_CERTPATH));
	return dir.absoluteFilePath ( file );
}

QString wwwFile(const QString& file) {
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_WWWPATH));
	return dir.absoluteFilePath ( file );
}

QDir serviceDir() {
    QDir dir = QDir::home();
	dir.mkdir(QLatin1String("roomcontrol"));
    dir.cd(QLatin1String("roomcontrol"));
	dir.mkdir(QLatin1String("services"));
    dir.cd(QLatin1String("services"));
    return dir;
}

QDir serviceBackupDir() {
    QDir dir = QDir::home();
	dir.mkdir(QLatin1String("roomcontrol"));
    dir.cd(QLatin1String("roomcontrol"));
	dir.mkdir(QLatin1String("backups"));
    dir.cd(QLatin1String("backups"));
    return dir;
}
