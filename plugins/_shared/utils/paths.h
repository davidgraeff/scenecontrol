#pragma once
#include <QStringList>
#include <QDir>

namespace setup {
void writeLastStarttime();

// Filepaths
QString certificateFile(const QString& file);
QString certificateFile(const char* file);
QStringList certificateClientFiles();

// Directory where all binaries of the plugins reside
QDir pluginDir(bool* found = 0);
// Directory where all initial json files reside
QDir dbimportDir(bool* found = 0);
// Directory where all imported/changed json files reside
QDir dbuserdir( bool* success );
// Directory where the application suite has been installed to.
QDir installdir();
};
