#pragma once
#include <QStringList>
#include <QDir>

// Filepaths
QString certificateFile(const QString& file);
QString wwwFile(const QString& file);

// Dirs
QDir pluginDir();

// Files
QStringList serverPluginsFiles();
QStringList serviceFiles();