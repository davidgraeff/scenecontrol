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
/** Returns json files and other attachment files that have to be installed to the couchDB for the plugin
  * to work correctly
  */
QDir pluginCouchDBDir();
QString couchdbAbsoluteUrl(const char* relativeUrl);
QString couchdbAbsoluteUrl(const QString& relativeUrl);
};
