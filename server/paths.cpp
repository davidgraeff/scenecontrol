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

QStringList serverPluginsFiles() {
	QDir dir = pluginDir();
	dir.cd(QLatin1String("server"));

	QStringList files = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
	for (int i=0;i<files.size();++i)
		files[i] = dir.absoluteFilePath ( files[i] );
	return files;
}


QStringList serviceFiles() {
	QStringList files;
	// append files in {home}/roomcontrol/services
	QDir dir = QDir::home();
	dir.cd(QLatin1String("roomcontrol"));
	dir.cd(QLatin1String("services"));
	dir.mkpath(dir.absolutePath());
	const QStringList tfiles = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
	for (int i=0;i<tfiles.size();++i)
		files.append(dir.absoluteFilePath ( tfiles[i] ));
	return files;
}
