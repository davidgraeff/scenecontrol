#pragma once
#include <QStringList>
#include <QDir>

QString certificateFile(const QString& file);

QDir pluginDir();

QStringList serverPluginsFiles();

QStringList clientPluginsFiles();

QStringList serviceFiles();