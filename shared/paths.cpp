#include "paths.h"

#include <QSettings>
#include "config.h"

QDir pluginDir() {
	QString defaultpath;
#ifdef _WIN32
	defaultpath = QLatin1String("");
#else
	defaultpath = QLatin1String("/usr/");
#endif
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_LIBPATH));
	return dir;
}

QString certificateFile(const QString& file) {
	QString defaultpath;
#ifdef _WIN32
	defaultpath = QLatin1String("");
#else
	defaultpath = QLatin1String("/usr/");
#endif
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_CERTPATH));
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

QStringList clientPluginsFiles() {
	QDir dir = pluginDir();
	dir.cd(QLatin1String("client"));

	QStringList files = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
	for (int i=0;i<files.size();++i)
		files[i] = dir.absoluteFilePath ( files[i] );
	return files;
}

QStringList serviceFiles() {
	QDir dir = QDir::home();
#ifdef _WIN32
	dir.cd(QLatin1String("roomcontrol"));
#else
	dir.cd(QLatin1String("roomcontrol"));
#endif
	dir.cd(QLatin1String("services"));
	QStringList files = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
	for (int i=0;i<files.size();++i)
		files[i] = dir.absoluteFilePath ( files[i] );
	return files;
}
