#include "paths.h"

#include <QSettings>
#include <QDateTime>

namespace setup {
  
#include "config.h"

#ifdef _WIN32
const QString defaultpath = QLatin1String("");
#else
const QString defaultpath = QLatin1String("/usr/");
#endif

QSettings settings(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String(ABOUT_ORGANIZATIONID));

void writeLastStarttime() {
  settings.setValue(QLatin1String("last"), QDateTime::currentDateTime().toString());
}

QDir pluginDir() {
    QDir dir(settings.value(QLatin1String("path"), defaultpath).toString());
    dir.cd(QLatin1String(ROOM_LIBPATH));
    return dir;
}

QString certificateFile(const QString& file) {
    QDir dir(settings.value(QLatin1String("path"), defaultpath).toString());
    dir.cd(QLatin1String(ROOM_CERTPATH));
    return dir.absoluteFilePath ( file );
}

QString certificateFile(const char* file) {
    return certificateFile(QString::fromUtf8(file));
}

QString wwwFile(const QString& file) {
    QDir dir(settings.value(QLatin1String("path"), defaultpath).toString());
    dir.cd(QLatin1String(ROOM_WWWPATH));
    return dir.absoluteFilePath ( file );
}

QString xmlFile(const QString& pluginid) {
    QDir dir(settings.value(QLatin1String("path"), defaultpath).toString());
    dir.cd(QLatin1String(ROOM_LIBPATH));
    dir.cd(pluginid);
    return dir.absoluteFilePath ( QLatin1String("plugin.xml") );
}

QString couchdbAbsoluteUrl(const char* relativeUrl) {
    QString basepath = settings.value(QLatin1String("couchdb"), QLatin1String(ROOM_COUCHDB)).toString();
    return basepath + QLatin1String("/") + QString::fromUtf8 ( relativeUrl );
}

};