#pragma once
#include <QStringList>
#include <QDir>

// Filepaths
QString certificateFile(const QString& file);
QString certificateFile(const char* file);
QString wwwFile(const QString& file);
QString xmlFile(const QString& pluginid);

// Dirs
QDir pluginDir();
QString couchdbAbsoluteUrl(const char* relativeUrl);
