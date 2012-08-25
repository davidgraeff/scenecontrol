#include "scenedocument.h"
#include "json.h"
#include <QDebug>

SceneDocument SceneDocument::createModelRemoveItem ( const char* id ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "remove" );
    return sc;
}

SceneDocument SceneDocument::createModelChangeItem ( const char* id ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "change" );
    return sc;
}

SceneDocument SceneDocument::createModelReset ( const char* id, const char* key ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__key" ) ] =  QLatin1String ( key );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "reset" );
    return sc;
}

SceneDocument SceneDocument::createNotification ( const char* id ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "notification" );
    return sc;
}

SceneDocument SceneDocument::createRemoveByUidCmd ( const QString& uid, const QString& type ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "__uid" ) ] = uid;
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "remove" );
	sc.m_map[QLatin1String ( "__oldtype" ) ] = type;
    return sc;
}

SceneDocument SceneDocument::createExecuteByUidCmd ( const QString& uid ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "__uid" ) ] = uid;
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "execute" );
    return sc;
}

SceneDocument SceneDocument::createExecuteByDataCmd ( const char* plugin_id, const char* id ) {
    SceneDocument sc;
    sc.m_map[QLatin1String ( "id" ) ] = QString ( QLatin1String ( plugin_id ) + QLatin1String ( "_" ) + QLatin1String ( id ) );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "execute" );
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
SceneDocument::SceneDocument(const QByteArray& jsondata) { m_map = JSON::parse(jsondata).toMap(); }
SceneDocument::SceneDocument(QTextStream& jsonstream) {
  bool error = false;
  m_map = JSON::parseValue(jsonstream, error).toMap();
  if (error) {
      qWarning() << "\tStream does not contain a json document!";
  }
}

bool SceneDocument::isValid() const {
  return !m_map.empty() && hasType() && hasid() && hasComponentID();
}

QByteArray SceneDocument::getjson() const { return JSON::stringify(m_map).toUtf8() + "\n"; }

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
