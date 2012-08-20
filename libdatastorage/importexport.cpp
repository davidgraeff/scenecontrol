#include <QDebug>
#include <QSocketNotifier>
#include <QTimer>
#include <QStringList>
#include <QDir>
#include <QHostInfo>
#include <QUuid>
#include <QUrl>
#include <qfile.h>

#include "shared/jsondocuments/scenedocument.h"
#include "shared/jsondocuments/json.h"
#include "datastorage.h"
#include "importexport.h"

#define __FUNCTION__ __FUNCTION__

namespace Datastorage {
	
void exportAsJSON(const DataStorage& ds, const QString& exportpath)
{
    QDir exportdir(exportpath);
    if (!exportdir.exists()) {
        qWarning() << "Database: failed to change to " << exportdir.absolutePath();
        return;
    }

    qDebug() << "Database: Export JSON Documents to" << exportpath;

	QDir datadir = ds.datadir();
	QStringList dirs;
	dirs.append(datadir.absolutePath());
	while (dirs.size()) {
		QDir currentdir(dirs.takeFirst());
		dirs.append(DataStorage::directories(currentdir));

		QString relpath(currentdir.absolutePath().mid(datadir.absolutePath().size()));
		
		QDir targetdir(exportpath+relpath);
		targetdir.mkpath(exportpath+relpath);

		QStringList files = currentdir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
		for (int i = 0; i < files.size(); ++i) {
			QFile::copy(datadir.absoluteFilePath(files[i]), targetdir.absoluteFilePath(files[i]));
		}
	}
}

void importFromJSON(DataStorage& ds, const QString& path, bool overwriteExisting, VerifyInterface* verify)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Database: failed to change to " << dir.absolutePath();
        return;
    }

    qDebug() << "Database: Import JSON Documents from" << path;

    const QStringList files = dir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
    for (int i = 0; i < files.size(); ++i) {
        QFile file(dir.absoluteFilePath(files[i]));
        file.open(QIODevice::ReadOnly);
        if (file.size() > 1024 * 10) {
            qWarning() << "\tFile to big!" << files[i] << file.size();
            continue;
        }
        QTextStream stream(&file);
        SceneDocument document(stream);
        if (!document.isValid()) {
            continue;
        }

		if (verify && !verify->isValid(document, files[i])) {
			qWarning() << "\tCustom verification denied import!";
            continue;
		}

        if (!document.hasType() || !document.hasPluginid()) {
            qWarning() << "\tType or plugin ID not set!";
        }

		if (!overwriteExisting && ds.contains(document))
			continue;

        ds.changeDocument(document, false);
    }

    // recursivly go into all subdirectories
    const QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.size(); ++i) {
        dir.cd(dirs[i]);
        importFromJSON(ds, dir.absolutePath(), overwriteExisting, verify);
        dir.cdUp();
    }
}

}