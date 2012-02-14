#include "pluginservicehelper.h"


ServiceData ServiceData::createModelRemoveItem ( const char* id ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "remove" );
    return sc;
}

ServiceData ServiceData::createModelChangeItem ( const char* id ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "change" );
    return sc;
}

ServiceData ServiceData::createModelReset ( const char* id, const char* key ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__key" ) ] =  QLatin1String ( key );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "reset" );
    return sc;
}

ServiceData ServiceData::createNotification ( const char* id ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "notification" );
    return sc;
}

ServiceData ServiceData::createRemoveByUidCmd ( const QString& uid, const QString& type ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "__uid" ) ] = uid;
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "remove" );
	sc.m_map[QLatin1String ( "__oldtype" ) ] = type;
    return sc;
}

ServiceData ServiceData::createExecuteByUidCmd ( const QString& uid ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "__uid" ) ] = uid;
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "execute" );
    return sc;
}

ServiceData ServiceData::createExecuteByDataCmd ( const char* plugin_id, const char* id ) {
    ServiceData sc;
    sc.m_map[QLatin1String ( "id" ) ] = QString ( QLatin1String ( plugin_id ) + QLatin1String ( "_" ) + QLatin1String ( id ) );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "execute" );
    return sc;
}

void ServiceData::setData ( const char* index, const QVariant& data ) {
    m_map[QLatin1String ( index ) ] = data;
}

QVariantMap& ServiceData::getData() {
    return m_map;
}
