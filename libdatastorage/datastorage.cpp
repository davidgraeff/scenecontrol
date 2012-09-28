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
#include "importexport.h"
#include "shared/utils/paths.h"

#define __FUNCTION__ __FUNCTION__

static DataStorage *databaseInstance = 0;

DataStorage::DataStorage() : m_listener(0), m_isLoading(false)
{
	// set directory
	bool found;
	m_dir = setup::dbuserdir(&found).absolutePath();
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
	if (!m_dir.exists() && !m_dir.mkpath(m_dir.absolutePath()))
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
	
	// enable the watcher
	m_listener->setEnabled(true);
	
	m_isLoading = true;
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
	m_isLoading = false;
	
	qDebug() << "Loaded actions:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeAction)).size();
	qDebug() << "Loaded conditions:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeCondition)).size();
	qDebug() << "Loaded events:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeEvent)).size();
	qDebug() << "Loaded scenes:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeScene)).size();
	qDebug() << "Loaded configurations:" << m_cache.value(SceneDocument::stringFromTypeEnum(SceneDocument::TypeConfiguration)).size();
}

QList< SceneDocument* > DataStorage::filterEntries(const QList< SceneDocument* >& source, const QVariantMap& filter) const {
	QList< SceneDocument* > d;
	// Go through all documents of source
	for(auto sourceIter = source.constBegin();sourceIter!=source.constEnd();++sourceIter) {
		// Assume document will be taken
		bool ok = true;
		// Loop through filter and check each filter entry with the current variantmap
		for(auto filterIter = filter.constBegin(); filterIter!= filter.constEnd(); ++filterIter) {
			if (filterIter.value() != (*sourceIter)->getData().value(filterIter.key())) {
				// If there is something in the filter (like plugin_=abc) and this is not matched by the current QVariantMap (like plugin_=def)
				// do not take this entry
				ok = false;
				break;
			}
		}
		if (ok)
			d.append(*sourceIter);
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

bool DataStorage::removeDocument(const SceneDocument &doc)
{
	if (!doc.isValid()) {
		qWarning() << "removeDocument: Not enough data provided to identify document to remove:" << doc.getjson();
		return false;
	}
	
	QFile d(storagePath(doc));
	if (!d.exists()) {
		qWarning() << "removeDocument: File does not exist" << d.fileName();
		return false;
	}
	
	if (!d.remove()) {
		qWarning() << "removeDocument: Remove failed" << d.fileName();
		return false;
	}
	
	// Remove related documents
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
	
	return true;
}

bool DataStorage::storeDocument(const SceneDocument& odoc, bool overwriteExisting)
{
	SceneDocument doc(odoc.getData());
	doc.checkIfIDneedsGUID();
	
	if (!doc.isValid()) {
		qWarning() << "storeDocument: Cannot store an invalid document";
		return false;
	}

	QFile f(storagePath(doc));
	if (f.exists() && !overwriteExisting)
		return true;
	
	// create dir if neccessary and add it to the watched directories
	QDir dir(QFileInfo(f.fileName()).absolutePath());
	if (!dir.exists()) {
		if (!dir.mkpath(dir.absolutePath())) {
			qWarning() << "storeDocument: Cannot create path" << dir.absolutePath();
			return false;
		}
		if (m_listener)
			m_listener->watchdir(dir.absolutePath());
	}
	
	// Write to disc
	if (!f.open(QFile::WriteOnly|QFile::Truncate)) {
		qWarning() << "storeDocument: could not open document for write";
		return false;
	}
	f.write(doc.getjson());
	f.close();

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
	
	if (filename != storagePath(*doc)) {
		qWarning() << "Storage location does not match content!" << filename;
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
	
	// Do not notify if we are still in the initial loading process currently
	if (m_isLoading)
		return;
	
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
void DataStorage::fetchAllDocuments(QList< SceneDocument >& result) const {
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

QString DataStorage::storagePath(const SceneDocument& doc) {
	return m_dir.absolutePath() + QLatin1String("/") + doc.type() + QLatin1String("/") + doc.componentID() + QLatin1String("/") + doc.filename();
}
