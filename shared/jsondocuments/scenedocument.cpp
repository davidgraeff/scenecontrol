#include "scenedocument.h"
#include "json.h"
#include <QDebug>
#include <QUuid>

SceneDocument SceneDocument::createModelRemoveItem ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(QLatin1String ( "model.remove" ));
    return sc;
}

SceneDocument SceneDocument::createModelChangeItem ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(QLatin1String ( "model.change" ));
    return sc;
}

SceneDocument SceneDocument::createModelReset ( const char* id, const char* key ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(QLatin1String ( "model.reset" ));
	sc.setModelkey(key);
    return sc;
}

SceneDocument SceneDocument::createNotification ( const char* id ) {
    SceneDocument sc;
	sc.setid(QLatin1String ( id ));
	sc.setType(QLatin1String ( "notification" ));
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

SceneDocument::SceneDocument(const QVariantMap& map) : m_map(map) {}
SceneDocument::SceneDocument(const QByteArray& jsondata) {
	JsonReader r;
	r.parse(jsondata);
	m_map = r.result().toMap();
}

bool SceneDocument::isValid() const {
  return !m_map.empty() && hasType() && hasid() && hasComponentID();
}

QByteArray SceneDocument::getjson() const {
	JsonWriter w;
	w.stringify(m_map);
	return w.result().toUtf8() + "\n";
}

bool SceneDocument::correctTypes(const QVariantMap& types)
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
