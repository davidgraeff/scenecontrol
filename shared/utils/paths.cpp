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

QDir pluginDir(bool* found) {
    QDir dir(settings.value(QLatin1String("path"), QLatin1String(ROOM_BASEPATH)).toString());
    if (!dir.cd(QLatin1String(ROOM_LIBPATH))) {
        qWarning() << "pluginDir path change failed";
        if (found)
            *found = false;
    } else
        if (found)
            *found = true;

    return dir;
}

QDir dbimportDir(bool* found)
{
    QDir dir(settings.value(QLatin1String("path"), QLatin1String(ROOM_BASEPATH)).toString());
    if (!dir.cd(QLatin1String(ROOM_DATABASEIMPORTPATH))) {
        qWarning() << "pluginDir path change failed";
        if (found)
            *found = false;
    } else
        if (found)
            *found = true;
    return dir;
}

QDir dbuserdir(bool createIfNotExists, bool* success)
{
	QSettings s(QSettings::IniFormat, QSettings::UserScope, QLatin1String(ABOUT_ORGANIZATIONID), QLatin1String(ABOUT_ORGANIZATIONID));
	QDir home = QFileInfo(s.fileName()).absoluteDir();
	*success = home.cd(QLatin1String("datastorage")) || (createIfNotExists && home.mkdir(QLatin1String("datastorage")) && home.cd(QLatin1String("datastorage")));
	return home;
}

QDir installdir() {
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
