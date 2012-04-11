#pragma once

#include <QObject>

class DatabaseInstall : public QObject
{
public:
    explicit DatabaseInstall(QObject* parent = 0);
    /**
     * Install missing json documents for the given plugin (synchronous)
     */
    bool installPlugindataIfMissing(const QString& pluginid, const QString& databaseImportPath);
};
