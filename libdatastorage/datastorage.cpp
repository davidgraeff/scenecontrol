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
	QSet<AbstractStorageNotifier*> copy = m_notifiers;
	qDeleteAll(copy);
	m_notifiers.clear();
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
}

void DataStorage::load()
{
	unload();
	if (!m_dir.exists() && !m_dir.mkpath(m_dir.absolutePath()))
		return;
	
	// Accepted documents for the cache
		m_cache.insert(SceneDocument::TypeAction,QList<SceneDocument*>());
		m_cache.insert(SceneDocument::TypeCondition,QList<SceneDocument*>());
		m_cache.insert(SceneDocument::TypeEvent,QList<SceneDocument*>());
		m_cache.insert(SceneDocument::TypeScene,QList<SceneDocument*>());
		m_cache.insert(SceneDocument::TypeConfiguration,QList<SceneDocument*>());

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
	
	qDebug() << "Loaded actions:" << m_cache.value(SceneDocument::TypeAction).size();
	qDebug() << "Loaded conditions:" << m_cache.value(SceneDocument::TypeCondition).size();
	qDebug() << "Loaded events:" << m_cache.value(SceneDocument::TypeEvent).size();
	qDebug() << "Loaded scenes:" << m_cache.value(SceneDocument::TypeScene).size();
	qDebug() << "Loaded configurations:" << m_cache.value(SceneDocument::TypeConfiguration).size();
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

QList<SceneDocument*> DataStorage::filteredDocuments(SceneDocument::TypeEnum type, const QVariantMap& filter) const {
	return filterEntries(m_cache.value(type), filter);
}

SceneDocument* DataStorage::getDocument(const QString& uid)
{
	return m_index_typeid.value(uid);
}

SceneDocument DataStorage::getDocumentCopy(const QString& uid)
{
	QMutexLocker locker(&mReadWriteLockMutex);
	SceneDocument* doc = m_index_typeid.value(uid);
	if (doc)
		return SceneDocument(doc->getData());
	else
		return SceneDocument();
}

int DataStorage::changeDocumentsValue(SceneDocument::TypeEnum type, const QVariantMap& filter, const QString& key, const QVariantMap& value) {
	QList<SceneDocument*> v = filterEntries(m_cache.value(type), filter);
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
	if (doc.isType(SceneDocument::TypeScene)) {
		const QString sceneid = doc.sceneid();
		QList<SceneDocument*> v;
		SceneDocument filterDoc;
		filterDoc.setSceneid(doc.id());
		
		v += filteredDocuments(SceneDocument::TypeAction, filterDoc.getData());
		v += filteredDocuments(SceneDocument::TypeCondition, filterDoc.getData());
		v += filteredDocuments(SceneDocument::TypeEvent, filterDoc.getData());
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
	SceneDocument* doc = new SceneDocument(file.readAll());
	file.close();
	
	if (!doc->isValid()) {
		qWarning() << "Document is not valid!" << doc->getjson();
		delete doc;
		return;
	}
	
	if (!m_cache.contains(doc->type())) {
		//qWarning() << "Document type not supported!" << doc->type();
		delete doc;
		return;
	}
	
	if (filename != storagePath(*doc)) {
		qWarning() << "Storage location does not match content!" << filename;
		delete doc;
		return;
	}
	
	QMutexLocker locker(&mReadWriteLockMutex);
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
	}
	
	m_index_filename.insert(filename, doc);
	m_index_typeid.insert(doc->uid(), doc);
	m_cache[doc->type()].append(doc);
	
	// Do not notify if we are still in the initial loading process currently
	if (!m_isLoading) {
		// notify StorageNotifiers
		for (auto it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
			(*it)->documentChanged(filename, olddoc, doc);
		}
	}

	// Delete old document
	delete olddoc;
}

void DataStorage::removeFromCache( const QString& filename ) {
	SceneDocument* olddoc = m_index_filename.take(filename);
	if (!olddoc)
		return;
	
	QMutexLocker locker(&mReadWriteLockMutex);
	m_index_typeid.remove(olddoc->uid());
	
	QMutableListIterator<SceneDocument*> i(m_cache[olddoc->type()]);
	while (i.hasNext()) {
		i.next();
		if (i.value() == olddoc) {
			i.remove();
			break;
		}
	}
	
	// notify StorageNotifiers
	for (auto it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
		(*it)->documentRemoved(filename, olddoc);
	}
	
	delete olddoc;
}
void DataStorage::fetchAllDocuments(QList< SceneDocument >& result) const {
	QStringList dirs;
	dirs.append(m_dir.absolutePath());	// add the user storage dir
	
	bool firstNotValidFlag = false;
	while (dirs.size()) {
		QDir currentdir(dirs.takeFirst());
		dirs.append(directories(currentdir));

		QStringList files = currentdir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
		for (int i = 0; i < files.size(); ++i) {
			QFile file(currentdir.absoluteFilePath(files[i]));
			file.open(QFile::ReadOnly);
			SceneDocument doc(file.readAll());
			file.close();
			
			if (!doc.isValid()) {
				if (!firstNotValidFlag) {
					firstNotValidFlag = true;
					qWarning() << "Invalid documents in" << currentdir.absolutePath();
				}
				qWarning() << "Document is not valid!" << files[i];
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
	return m_dir.absolutePath() + QLatin1String("/") + SceneDocument::typeString(doc.type()) + QLatin1String("/") + doc.componentID() + QLatin1String("/") + doc.filename();
}
