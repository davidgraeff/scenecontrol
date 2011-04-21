#pragma once
#include <QStringList>
#include <QDir>

// Filepaths
QString certificateFile(const QString& file);
QString wwwFile(const QString& file);
QString xmlFile(const QString& pluginid);

// Dirs
QDir pluginDir();
QDir serviceDir();
QDir serviceBackupDir();
