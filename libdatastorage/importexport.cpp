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
	
int exportAsJSON(const DataStorage& ds, const QString& exportpath, bool overwriteExisting)
{
    QDir exportdir(exportpath);
    if (!exportdir.exists() && !exportdir.mkpath(exportdir.absolutePath())) {
        qWarning() << "Database: failed to change to " << exportdir.absolutePath();
        return 0;
    }

    int counter = 0;
	QDir datadir = ds.datadir();
	QStringList dirs;
	dirs.append(datadir.absolutePath());
	while (dirs.size()) {
		QDir currentdir(dirs.takeFirst());
		dirs.append(DataStorage::directories(currentdir));

		QString relpath(currentdir.absolutePath().mid(datadir.absolutePath().size()));
		
		QDir targetdir(exportdir.absolutePath()+QLatin1String("/")+relpath);
		targetdir.mkpath(targetdir.absolutePath());

		QStringList files = currentdir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
		for (int i = 0; i < files.size(); ++i) {
			if (overwriteExisting && targetdir.exists(files[i]))
				targetdir.remove(files[i]);
			if (QFile::copy(currentdir.absoluteFilePath(files[i]), targetdir.absoluteFilePath(files[i])))
				++counter;
		}
	}
	
	return counter;
}

int importFromJSON(DataStorage& ds, const QString& path, bool overwriteExisting, VerifyInterface* verify)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Database: failed to change to " << dir.absolutePath();
        return 0;
    }
    
    int counter = 0;
	
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
		++counter;
    }

    // recursivly go into all subdirectories
    const QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.size(); ++i) {
        dir.cd(dirs[i]);
        counter += importFromJSON(ds, dir.absolutePath(), overwriteExisting, verify);
        dir.cdUp();
    }
    
    return counter;
}

bool VerifyImportDocument::isValid(SceneDocument& data, const QString& filename) {
	const QString subdir = QFileInfo(filename).absoluteDir().dirName(); //componentid: dir name
	if (!data.hasComponentID())
		data.setComponentID(subdir);
	if (!data.hasid()) // no identifier set?: use filename without extension and componentid
		data.setid(QFileInfo(filename).completeBaseName());
	if (data.checkType(SceneDocument::TypeConfiguration) && !data.hasComponentUniqueID()) // configurations need an instanceid
		return false;
	return true;
}

bool VerifyPluginDocument::isValid(SceneDocument& data, const QString& filename) {
	if (!data.hasComponentID())
		data.setComponentID(m_pluginid);
	if (!data.hasid()) // no identifier set? use filename without extension and componentid
		data.setid(QFileInfo(filename).completeBaseName());
	if (data.checkType(SceneDocument::TypeConfiguration) && !data.hasComponentUniqueID()) // configurations need an instanceid
		return false;
	return true;
}

}