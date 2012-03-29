#include "paths.h"

#include <QSettings>
#include <QDateTime>
#include <QDebug>

namespace setup {

#include "config.h"

QSettings settings(QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String(ABOUT_ORGANIZATIONID));

void writeLastStarttime() {
    settings.setValue(QLatin1String("last"), QDateTime::currentDateTime().toString());
}

QDir pluginDir() {
    QDir dir(settings.value(QLatin1String("path"), QLatin1String(ROOM_BASEPATH)).toString());
    if (!dir.cd(QLatin1String(ROOM_LIBPATH))) {
      qWarning() << "pluginDir path change failed";
    }
    return dir;
}

QDir baseDir() {
    QDir dir(settings.value(QLatin1String("path"), QLatin1String(ROOM_BASEPATH)).toString());
    return dir;
}


QString certificateFile(const QString& file) {
    QDir dir(settings.value(QLatin1String("path"), QLatin1String(ROOM_BASEPATH)).toString());
    dir.cd(QLatin1String(ROOM_CERTPATH));
    return dir.absoluteFilePath ( file );
}

QString certificateFile(const char* file) {
    return certificateFile(QString::fromUtf8(file));
}

};
