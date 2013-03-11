#include "scenedocument.h"
#include "json.h"
#include <QDebug>
#include <QUuid>
#include <stdarg.h>

const char* const SceneDocument::typetext[] = {" ",
"event","condition","action","scene","configuration","schema",
"execute","remove","notification",
"model","error","ack","auth"," "};

SceneDocument SceneDocument::createModelRemoveItem ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeModelItem);
	sc.setData("action","remove");
    return sc;
}

SceneDocument SceneDocument::createModelChangeItem ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeModelItem);
	sc.setData("action","change");
    return sc;
}

SceneDocument SceneDocument::createModelReset ( const char* id, const char* key ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeModelItem);
	sc.setData("action","reset");
	sc.setModelkey(key);
    return sc;
}

SceneDocument SceneDocument::createNotification ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeNotification);
    return sc;
}

void SceneDocument::setData ( const char* index, const QVariant& data ) {
    m_map[QLatin1String ( index ) ] = data;
}

QVariantMap& SceneDocument::getData() {
    return m_map;
}
const QVariantMap& SceneDocument::getData() const {
    return m_map;
}
QString SceneDocument::toString(const char* key) const {
    return m_map.value(QString::fromUtf8(key)).toString();
}
int SceneDocument::toInt(const char* key) const {
    return m_map.value(QString::fromUtf8(key)).toInt();
}
bool SceneDocument::toBool(const char* key) const {
    return m_map.value(QString::fromUtf8(key)).toBool();
}
double SceneDocument::toDouble(const char* key) const {
    return m_map.value(QString::fromUtf8(key)).toDouble();
}
QVariantList SceneDocument::toList(const char* key) const {
    return m_map.value(QString::fromUtf8(key)).toList();
}
QVariantMap SceneDocument::toMap(const char* key) const {
    return m_map.value(QString::fromUtf8(key)).toMap();
}

SceneDocument::SceneDocument(const QVariantMap& map) : m_map(map) {convertType();}
SceneDocument::SceneDocument(const QByteArray& jsondata, const QByteArray& hash) {
	mHash = hash;
	JsonReader r;
	r.parse(jsondata);
	m_map = r.result().toMap();
	convertType();
}
SceneDocument::SceneDocument(const QByteArray& jsondata)
{
	JsonReader r;
	r.parse(jsondata);
	m_map = r.result().toMap();
	convertType();
}

void SceneDocument::convertType()
{
	QByteArray type = m_map.value(QLatin1String("type_")).toByteArray();
	mType = TypeUnknown;
	for (int i=0;i<TypeLAST;++i) {
		if (strlen(typetext[i])==type.size() && strncmp(type,typetext[i],type.size())==0) {
			mType = (TypeEnum)i;
			break;
		}
	}
}
QString SceneDocument::typeString(const SceneDocument::TypeEnum t)
{
	return QString::fromAscii(typetext[t]);
}


bool SceneDocument::isValid() const {
  return !m_map.empty() && hasType() && hasid() && hasComponentID();
}

QByteArray SceneDocument::getjson() const {
	JsonWriter w;
	w.stringify(m_map);
	return w.result().toUtf8() + "\n";
}

bool SceneDocument::correctDataTypes(const QVariantMap& types)
{
    if (types.empty())
      return true;
    
    QVariantMap result;
    // convert types if neccessary (javascript/qt-qml do not use the same types as QVariant some times)
    QVariantMap::const_iterator i = m_map.begin();
    for (;i!=m_map.end();++i) {
        const QByteArray targettype = types.value(i.key()).toByteArray();
        if (targettype.isEmpty()) {
            result[i.key()] = i.value();
            continue;
        }
        QVariant element = i.value();
        if (element.type()!=QVariant::nameToType(targettype) && !element.convert(QVariant::nameToType(targettype))) {
            qWarning()<<"checkTypes: Conversion failed" << i.key() << "orig:" << i.value().typeName() << "dest:" << targettype;
            continue;
        }
        result[i.key()] = element;
    }

    if (result.size() < m_map.size())
        return false;
    m_map = result;
	return true;
}

void SceneDocument::checkIfIDneedsGUID() {
	if (id()==QLatin1String("GENERATEGUID"))
		setid(QUuid::createUuid().toString().replace(QLatin1String("{"),QString()).replace(QLatin1String("}"),QString()));
}
QString SceneDocument::id() const {
    return m_map.value(QLatin1String("id_")).toString();
}
QString SceneDocument::uid() const {
    return m_map.value(QLatin1String("type_")).toString()+m_map.value(QLatin1String("id_")).toString();
}
QString SceneDocument::uid(SceneDocument::TypeEnum type, const QString& id) {
	return typeString(type)+id;
}
QString SceneDocument::id(const QVariantMap& data) {
    return data.value(QLatin1String("id_")).toString();
}
QString SceneDocument::idkey() { return QLatin1String("id_"); }
void SceneDocument::setid(const QString& id) {
  m_map[QLatin1String("id_")] = id;
}
bool SceneDocument::hasid() const {
    return m_map.contains(QLatin1String("id_"));
}
QString SceneDocument::componentID() const {
    return m_map.value(QLatin1String("componentid_")).toString();
}
void SceneDocument::setComponentID(const QString& pluginid) {
    m_map[QLatin1String("componentid_")] = pluginid;
}
bool SceneDocument::hasComponentID() const {
	return m_map.contains(QLatin1String("componentid_"));
}
bool SceneDocument::hasComponentUniqueID() const {
	return m_map.contains(QLatin1String("componentid_")) && m_map.contains(QLatin1String("instanceid_"));
}
QString SceneDocument::componentUniqueID() const {
    return m_map.value(QLatin1String("componentid_")).toString()+m_map[QLatin1String("instanceid_")].toString();
}
QString SceneDocument::instanceID() const {
    return m_map[QLatin1String("instanceid_")].toString();
}
void SceneDocument::setInstanceID(const QString& instanceid) {
    m_map[QLatin1String("instanceid_")] = instanceid;
}
void SceneDocument::setType(SceneDocument::TypeEnum t) {
	mType = t;
	m_map[QLatin1String("type_")] = QByteArray(typetext[t]);
}
SceneDocument::TypeEnum SceneDocument::type() const {
return mType;
  }
bool SceneDocument::hasType() const {
    return mType!=TypeUnknown;
}
QString SceneDocument::sceneid() const {
    return m_map.value(QLatin1String("sceneid_")).toString();
}
void SceneDocument::setSceneid(const QString& collectionid) {
    m_map[QLatin1String("sceneid_")] = collectionid;
}
QByteArray SceneDocument::modelkey() const {
    return m_map.value(QLatin1String("key_")).toByteArray();
}
void SceneDocument::setModelkey(const QByteArray& configurationkey) {
    m_map[QLatin1String("key_")] = configurationkey;
}
QString SceneDocument::filename() const {return id() + QLatin1String(".json"); }
bool SceneDocument::isMethod(const char* id) const {
    return m_map.value(QLatin1String("method_")).toByteArray() == QByteArray(id);
}
QByteArray SceneDocument::method() const {
    return m_map.value(QLatin1String("method_")).toByteArray();
}
void SceneDocument::setMethod(const QByteArray& methodname) {
    m_map[QLatin1String("method_")] = methodname;
}
bool SceneDocument::hasMethod() const {
    return m_map.contains(QLatin1String("method_"));
}
int SceneDocument::sessionid() const {
    return m_map.value(QLatin1String("sessionid_"),-1).toInt();
}
void SceneDocument::removeSessionID() {
    m_map.remove(QLatin1String("sessionid_"));
}
void SceneDocument::setSessionID(const int sessionid) {
    m_map[QLatin1String("sessionid_")] = sessionid;
}
QVariantList SceneDocument::nextNodes() const {
	return m_map.value(QLatin1String("e")).toList();
}
void SceneDocument::setNextNodes(const QVariantList& nextNodes) {
	m_map[QLatin1String("e")] = nextNodes;
}
QVariantList SceneDocument::nextAlternativeNodes() const {
	return m_map.value(QLatin1String("eAlt")).toList();
}
void SceneDocument::setAlternativeNextNodes(const QVariantList& nextNodes) {
	m_map[QLatin1String("eAlt")] = nextNodes;
}
QVariantList SceneDocument::sceneItems() const {
	return m_map.value(QLatin1String("v")).toList();
}
void SceneDocument::setSceneItems(const QVariantList& sceneitemList)
{
	m_map[QLatin1String("v")] = sceneitemList;
}
void SceneDocument::addSceneItem(SceneDocument* sceneItemDoc)
{
	QVariantList v = m_map.value(QLatin1String("v")).toList();
	QVariantMap sceneItem;
	sceneItem[QLatin1String("id_")] = sceneItemDoc->id();
	sceneItem[QLatin1String("type_")] = typeString(sceneItemDoc->type());
	v.append(sceneItem);
	m_map[QLatin1String("v")] = v;
}
void SceneDocument::removeSceneItem(SceneDocument* sceneItemDoc)
{
	QVariantList v = m_map.value(QLatin1String("v")).toList();
	for (int i=v.size()-1;i>=0;--i) {
		const QVariantMap sceneItem = v[i].toMap();
		const QString id = sceneItem.value(QLatin1String("id_")).toString();
		const QString type_ = sceneItem.value(QLatin1String("type_")).toString();
		if (id == sceneItemDoc->id() && (type_.isEmpty() || type_ == typeString(sceneItemDoc->type()))) {
			v.removeAt(i);
		}
	}
	m_map[QLatin1String("v")] = v;
}

const QByteArray SceneDocument::getHash() {return mHash;}
bool SceneDocument::isType(const SceneDocument::TypeEnum t) const {
	return (t==mType);
}
bool SceneDocument::isOneOfType(int typeArraySize, ...) const {
	va_list params; // Zugriffshandle für Parameter
	SceneDocument::TypeEnum par;     // Parameterinhalt
	va_start(params, typeArraySize); // Zugriff vorbereiten

	bool r = false;
	for(int i=0;i<typeArraySize;++i) {
		par = (SceneDocument::TypeEnum)va_arg(params, int); // hole den Parameter
		if (par==mType) { r = true; break; }
	}
	
	va_end(params); // Zugriff abschließen
	return r;
}
