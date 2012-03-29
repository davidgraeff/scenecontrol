#pragma once
#include <QStringList>
#include <QDir>

namespace setup {
void writeLastStarttime();

// Filepaths
QString certificateFile(const QString& file);
QString certificateFile(const char* file);

// Dirs
QDir pluginDir();
QDir baseDir();
};
