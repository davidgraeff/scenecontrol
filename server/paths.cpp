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

QString certificateFile(const char* file) {
  return certificateFile(QString::fromUtf8(file));
}

QString wwwFile(const QString& file) {
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_WWWPATH));
	return dir.absoluteFilePath ( file );
}

QString xmlFile(const QString& pluginid) {
	QSettings s(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String("main"));
	QDir dir(s.value(QLatin1String("path"), defaultpath).toString());
	dir.cd(QLatin1String(ROOM_LIBPATH));
	dir.cd(pluginid);
	return dir.absoluteFilePath ( QLatin1String("plugin.xml") );
}

QString couchdbAbsoluteUrl(const char* relativeUrl) {
    return QLatin1String(ROOM_COUCHDB) + QString::fromUtf8 ( relativeUrl );
}

