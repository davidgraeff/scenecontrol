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

		if (verify && !verify->isValid(document, dir.absoluteFilePath(files[i]))) {
			qWarning() << "Custom verification denied import!" << files[i];
            continue;
		}

        if (!document.hasType()) {
            qWarning() << "Type not set:" << files[i];
			continue;
        }
        if (!document.hasid()) {
            qWarning() << "ID not set:" << files[i];
			continue;
        }
        if (!document.hasComponentID()) {
            qWarning() << "ComponentID not set:" << files[i];
			continue;
        }

        ds.storeDocument(document, overwriteExisting);
    }

    // recursivly go into all subdirectories
    const QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.size(); ++i) {
        dir.cd(dirs[i]);
        importFromJSON(ds, dir.absolutePath(), overwriteExisting, verify);
        dir.cdUp();
    }
}

bool VerifyImportDocument::isValid(SceneDocument& data, const QString& filename) {
	QFileInfo finfo(filename);
	if (!data.hasComponentID())
		data.setComponentID(finfo.absoluteDir().dirName()); //componentid: dir name
	if (!data.hasid()) // no identifier set?: use filename without extension and componentid
		data.setid(QFileInfo(filename).completeBaseName() + QLatin1String(".") + finfo.absoluteDir().dirName());
	if (data.checkType(SceneDocument::TypeConfiguration) && !data.hasComponentUniqueID()) // configurations need an instanceid
		return false;
	return true;
}

bool VerifyPluginDocument::isValid(SceneDocument& data, const QString& filename) {
	if (!data.hasComponentID())
		data.setComponentID(m_pluginid);
	if (!data.hasid()) // no identifier set? use filename without extension and componentid
          	data.setid((QString)(QFileInfo(filename).completeBaseName() + QLatin1String(".") + m_pluginid));
	if (data.checkType(SceneDocument::TypeConfiguration) && !data.hasComponentUniqueID()) // configurations need an instanceid
		return false;
	return true;
}

}