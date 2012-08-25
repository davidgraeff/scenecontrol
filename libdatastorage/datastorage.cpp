#include "datastorage.h"

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
#include "databaselistener.h"
#include "shared/utils/paths.h"

#define __FUNCTION__ __FUNCTION__

static DataStorage *databaseInstance = 0;

DataStorage::DataStorage() : m_listener(0)
{
	// set directory
	bool found;
	m_dir = setup::dbuserdir(true, &found).absolutePath();
}

DataStorage::~DataStorage()
{
	unload();
}

DataStorage *DataStorage::instance()
{
	if (databaseInstance == 0)
		databaseInstance = new DataStorage();
	
	return databaseInstance;
}

QDir DataStorage::datadir() const {return m_dir;}

void DataStorage::unload()
{
	qDeleteAll(m_index_typeid);
	m_index_typeid.clear();
	delete m_listener;
	m_listener = 0;
	m_cache.clear();
	qDeleteAll(m_notifiers);
	m_notifiers.clear();
}

void DataStorage::load()
{
	unload();
	if (!m_dir.exists())
		return;

	m_cache.insert(QLatin1String("action"), QList<SceneDocument*>());
	m_cache.insert(QLatin1String("event"), QList<SceneDocument*>());
	m_cache.insert(QLatin1String("condition"), QList<SceneDocument*>());
	m_cache.insert(QLatin1String("scene"), QList<SceneDocument*>());
	m_cache.insert(QLatin1String("configuration"), QList<SceneDocument*>());
	
	// initiate a directory watcher
	m_listener = new DataStorageWatcher();
	connect(m_listener, SIGNAL(fileChanged(QString)), SLOT(reloadDocument(QString)));
	connect(m_listener, SIGNAL(fileRemoved(QString)), SLOT(removeFromCache(QString)));
	
	QStringList dirs;
	dirs.append(m_dir.absolutePath());	// add the user storage dir
	
	while (dirs.size()) {
		QDir currentdir(dirs.takeFirst());
		m_listener->watchdir(currentdir.absolutePath());
		dirs.append(directories(currentdir));

		QStringList files = currentdir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
		for (int i = 0; i < files.size(); ++i) {
			reloadDocument(currentdir.absoluteFilePath(files[i]));
		}
	}
	
	qDebug() << "Loaded actions:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeAction)).size();
	qDebug() << "Loaded conditions:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeCondition)).size();
	qDebug() << "Loaded events:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeEvent)).size();
	qDebug() << "Loaded scenes:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeScene)).size();
	qDebug() << "Loaded configurations:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeConfiguration)).size();
}

QList< SceneDocument* > DataStorage::filterEntries(const QList< SceneDocument* >& source, const QVariantMap& filter) const {
	QList< SceneDocument* > d;
	QList< SceneDocument* >::const_iterator it = source.constBegin();
	// Go through all QVariantMaps of source
	for(;it!=source.constEnd();++it) {
		// Assume entry will be taken
		bool ok = true;
		// Loop through filter and check each filter entry with the current variantmap
		for(auto i = filter.constBegin();i!= filter.constEnd();++i) {
			if ((*it)->getData().value(i.key()) != i.value()) {
				// If there is something in the filter (like plugin_=abc) and this is not matched by the current QVariantMap (like plugin_=def)
				// do not take this entry
				ok = false;
				break;
			}
		}
		if (!ok)
			continue;
		d.append(*it);
	}
	return d;
}

QStringList DataStorage::directories(const QDir& dir) {
	QStringList d = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < d.size(); ++i) {
		d[i] = dir.absoluteFilePath(d[i]);
	}
	return d;
}

QList<SceneDocument*> DataStorage::requestAllOfType(SceneDocument::TypeEnum type, const QVariantMap& filter) const {
	// special if id is known
	const QString typetext = SceneDocument::stringFromTypeEnum(type);
	if (filter.contains(SceneDocument::idkey()))
		return m_index_typeid.values(SceneDocument::id(filter)+typetext);
	// generic
		return filterEntries(m_cache.value(SceneDocument::stringFromTypeEnum(type)), filter);
}

int DataStorage::changeDocumentsValue(SceneDocument::TypeEnum type, const QVariantMap& filter, const QString& key, const QVariantMap& value) {
	QList<SceneDocument*> v = filterEntries(m_cache.value(SceneDocument::stringFromTypeEnum(type)), filter);
	for (int i=0;i<v.size();++i) {
		v[i]->getData().insert(key, value); 
		storeDocument(*v[i], true);
	}
	return v.size();
}

void DataStorage::removeDocument(const SceneDocument &doc)
{
	if (!doc.isValid()) {
		qWarning() << "changeDocument: can not add/change an invalid document";
		return;
	}
	
	// remove from disc
	QDir d(m_dir);
	m_dir.cd(doc.type());
	if (!QFile::remove(d.absoluteFilePath(doc.filename()))) {
		qWarning() << "removeDocument: can not remove document" << d.absoluteFilePath(doc.filename());
		return;
	}
	
	if (doc.checkType(SceneDocument::TypeScene)) {
		const QString sceneid = doc.sceneid();
		QList<SceneDocument*> v;
		
		v = m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeAction));
		for (int i=v.size()-1;i>=0;--i) removeDocument(*v[i]);
		v = m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeCondition));
		for (int i=v.size()-1;i>=0;--i) removeDocument(*v[i]);
		v = m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeEvent));
		for (int i=v.size()-1;i>=0;--i) removeDocument(*v[i]);
	}
}

bool DataStorage::storeDocument(const SceneDocument& doc, bool overwriteExisting)
{
	if (!doc.isValid()) {
		qWarning() << "storeDocument: can not store an invalid document";
		return false;
	}
	
	// Write to disc
	QDir d(m_dir);
	if (!d.cd(doc.type()) && !(d.mkdir(doc.type()) && d.cd(doc.type()))) {
		qWarning() << "storeDocument: could not open subdir" << doc.type();
		return false;
	}
	m_listener->watchdir(doc.type());
	
	QFile f(d.absoluteFilePath(doc.filename()));
	
	if (f.exists() && !overwriteExisting)
		return true;
	
	if (!f.open(QFile::WriteOnly|QFile::Truncate)) {
		qWarning() << "storeDocument: could not open document for write";
		return false;
	}
	f.write(doc.getjson());
	f.close();
		
	// This should not be neccessary, all storage paths signal changes
// 		removeFromCache(doc.type(), doc.id());
// 		SceneDocument* ndoc = new SceneDocument(doc.getjson());
// 		this->updateCache(ndoc);
	
	return true;
}

bool DataStorage::contains(const SceneDocument& doc) const
{
	return m_index_typeid.contains(doc.uid());
}

void DataStorage::reloadDocument( const QString& filename ) {
	QFile file(filename);
	file.open(QFile::ReadOnly);
	QTextStream stream(&file);
	SceneDocument* doc = new SceneDocument(stream);
	file.close();
	
	if (!doc->isValid()) {
		qWarning() << "Document is not valid!" << filename;
		delete doc;
		return;
	}
	
	if (!m_cache.contains(doc->type())) {
		delete doc;
		return;
	}
	
	SceneDocument* olddoc = m_index_filename.take(filename);
	if (olddoc) {
		m_index_typeid.remove(olddoc->uid());
		QMutableListIterator<SceneDocument*> i(m_cache[doc->type()]);
		while (i.hasNext()) {
			i.next();
			if (i.value() == olddoc) {
				i.remove();
				break;
			}
		}
		delete olddoc;
	}
	
	m_index_filename.insert(filename, doc);
	m_index_typeid.insert(doc->uid(), doc);
	m_cache[doc->type()].append(doc);
	emit doc_changed(doc);
	
	// notify StorageNotifiers
	for (auto it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
		(*it)->documentChanged(filename, doc);
	}
}

void DataStorage::removeFromCache( const QString& filename ) {
	SceneDocument* olddoc = m_index_filename.take(filename);
	if (!olddoc)
		return;
	m_index_typeid.remove(olddoc->uid());
	
	QMutableListIterator<SceneDocument*> i(m_cache[olddoc->type()]);
	while (i.hasNext()) {
		i.next();
		if (i.value() == olddoc) {
			i.remove();
			break;
		}
	}
	
	emit doc_removed(olddoc);
	
	// notify StorageNotifiers
	for (auto it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
		(*it)->documentRemoved(filename, olddoc);
	}
	
	delete olddoc;
}
const QList< SceneDocument >& DataStorage::fetchAllDocuments() const {
	QList< SceneDocument > result;
	
	QStringList dirs;
	dirs.append(m_dir.absolutePath());	// add the user storage dir
	
	while (dirs.size()) {
		QDir currentdir(dirs.takeFirst());
		dirs.append(directories(currentdir));

		QStringList files = currentdir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
		for (int i = 0; i < files.size(); ++i) {
			QFile file(currentdir.absoluteFilePath(files[i]));
			file.open(QFile::ReadOnly);
			QTextStream stream(&file);
			SceneDocument doc(stream);
			file.close();
			
			if (!doc.isValid()) {
				qWarning() << "Document is not valid!" << currentdir.absoluteFilePath(files[i]);
				continue;
			}

			result.append(doc);
		}
	}
	
	return result;
}

void DataStorage::registerNotifier ( AbstractStorageNotifier* notifier ) {
	Q_ASSERT(notifier);
	m_notifiers.insert(notifier);
	// Autoremove the notifier if it gets deleted
	connect(notifier, SIGNAL(destroyed(QObject*)), SLOT(unregisterNotifier(QObject*)));
}

void DataStorage::unregisterNotifier ( QObject* obj ) {
	m_notifiers.remove((AbstractStorageNotifier*)obj);
}
