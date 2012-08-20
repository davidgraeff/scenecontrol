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

#define __FUNCTION__ __FUNCTION__

static DataStorage *databaseInstance = 0;

DataStorage::DataStorage() : m_listener(0)
{
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
}

void DataStorage::load()
{
	unload();
	m_listener = new DataStorageWatcher();
	connect(m_listener, SIGNAL(doc_changed(SceneDocument*)), SLOT(updateCache(SceneDocument*)));
	connect(m_listener, SIGNAL(doc_removed(QString,QString)), SLOT(removeFromCache(QString,QString)));
	
	QStringList dirs;
	dirs.append(m_dir.absolutePath());
	while (dirs.size()) {
		QDir currentdir(dirs.takeFirst());
		m_listener->watchdir(currentdir.absolutePath());
		dirs.append(directories(currentdir));

		QStringList files = currentdir.entryList(QStringList(QLatin1String("*.json")), QDir::Files | QDir::NoDotAndDotDot);
		for (int i = 0; i < files.size(); ++i) {
			QFile file(currentdir.absoluteFilePath(files[i]));
			file.open(QFile::ReadOnly);
			QTextStream stream(&file);
			SceneDocument* doc = new SceneDocument(stream);
			file.close();
			updateCache(doc);
		}
	}
}

QList< SceneDocument* > DataStorage::filterEntries(const QList< SceneDocument* >& source, const QVariantMap& filter) const {
	QList< SceneDocument* > d;
	QList< SceneDocument* >::const_iterator it = source.constBegin();
	// Go through all QVariantMaps of source
	for(;it!=source.constEnd();++it) {
		// Assume entry will be taken
		bool ok = true;
		// Loop through filter and check each filter entry with the current variantmap
		QVariantMap::const_iterator i = filter.constBegin();
		for(;i!= filter.constEnd();++i) {
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
		changeDocument(*v[i]);
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

bool DataStorage::changeDocument(const SceneDocument& doc, bool insertWithNewID, const QVariantMap& types)
{
	if (!doc.isValid()) {
		qWarning() << "changeDocument: can not add/change an invalid document";
		return false;
	}
	
	// Add id if none present
	SceneDocument mdoc(doc);
	if (!doc.hasid()) {
		if (insertWithNewID)
			mdoc.setid(QUuid::createUuid().toString().
			replace(QLatin1String("{"),QString()).
			replace(QLatin1String("}"),QString()).
			replace(QLatin1String("-"),QString()));
		else {
			qWarning() << "changeDocument: can not change document without _id";
			return false;
		}
	}
	
	// Correct types
	if (!mdoc.correctTypes(types)) {
		qWarning() << "changeDocument: correctTypes failed";
		return false;
	}
	
	// Write to disc
	QDir d(m_dir);
	m_dir.cd(doc.type());
	QFile f(d.absoluteFilePath(doc.filename()));
	if (!f.open(QFile::WriteOnly|QFile::Truncate)) {
		qWarning() << "changeDocument: could not open document for write";
		return false;
	}
	f.write(mdoc.getjson());
	f.close();
	
	return true;
}

bool DataStorage::contains(const SceneDocument& doc) const
{
	return m_index_typeid.contains(doc.uid());
}

void DataStorage::updateCache(SceneDocument* doc) {
	if (!m_cache.contains(doc->type()))
		return;
		
	SceneDocument* olddoc = m_index_typeid.take(doc->uid());
	m_index_typeid.insert(doc->uid(), doc);
	
	QMutableListIterator<SceneDocument*> i(m_cache[doc->type()]);
	while (i.hasNext()) {
		i.next();
		if (i.value() == olddoc) {
			i.remove();
			break;
		}
	}
	
	delete olddoc;
	m_cache[doc->type()].append(doc);
	emit doc_changed(*doc);
}

void DataStorage::removeFromCache(const QString& type, const QString& id) {
	if (!m_cache.contains(type))
		return;
	
	SceneDocument* doc = m_index_typeid.take(type+id);
	
	QMutableListIterator<SceneDocument*> i(m_cache[type]);
	while (i.hasNext()) {
		i.next();
		if (i.value() == doc) {
			i.remove();
			break;
		}
	}
	
	emit doc_removed(*doc);
	delete doc;
}
