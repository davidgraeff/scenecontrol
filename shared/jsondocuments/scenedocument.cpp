#include "scenedocument.h"
#include "json.h"
#include <QDebug>
#include <QUuid>

const char* const SceneDocument::typetext[] = {" ",
"event","condition","action","scene","configuration","schema",
"execute","remove","notification",
"model","model.remove","model.change","model.reset",
"error"," "};

SceneDocument SceneDocument::createModelRemoveItem ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeModelItemRemove);
    return sc;
}

SceneDocument SceneDocument::createModelChangeItem ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeModelItemChange);
    return sc;
}

SceneDocument SceneDocument::createModelReset ( const char* id, const char* key ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(TypeModelItemReset);
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
SceneDocument::SceneDocument(const QByteArray& jsondata) {
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
