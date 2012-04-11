#pragma once
#include <QStringList>
#include <QDir>

namespace setup {
void writeLastStarttime();

// Filepaths
QString certificateFile(const QString& file);
QString certificateFile(const char* file);

// Dirs
QDir pluginDir(bool* found = 0);
QDir dbimportDir(bool* found = 0);
QDir baseDir();
};
