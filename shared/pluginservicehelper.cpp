#include "pluginservicehelper.h"


ServiceCreation ServiceCreation::createModelRemoveItem ( const char* plugin_id, const char* id ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__plugin" ) ] = QLatin1String ( plugin_id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "remove" );
    return sc;
}

ServiceCreation ServiceCreation::createModelChangeItem ( const char* plugin_id, const char* id ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__plugin" ) ] = QLatin1String ( plugin_id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "change" );
    return sc;
}

ServiceCreation ServiceCreation::createModelReset ( const char* plugin_id, const char* id, const char* key ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__key" ) ] =  QLatin1String ( key );
    sc.m_map[QLatin1String ( "__plugin" ) ] = QLatin1String ( plugin_id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "model" );
    sc.m_map[QLatin1String ( "__event" ) ] = QLatin1String ( "reset" );
    return sc;
}

ServiceCreation ServiceCreation::createNotification ( const char* plugin_id, const char* id ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "id" ) ] =  QLatin1String ( id );
    sc.m_map[QLatin1String ( "__plugin" ) ] = QLatin1String ( plugin_id );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "notification" );
    return sc;
}

ServiceCreation ServiceCreation::createRemoveByUidCmd ( const QString& uid, const QString& type ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "__uid" ) ] = uid;
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "remove" );
	sc.m_map[QLatin1String ( "__oldtype" ) ] = type;
    return sc;
}

ServiceCreation ServiceCreation::createExecuteByUidCmd ( const QString& uid ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "__uid" ) ] = uid;
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "execute" );
    return sc;
}

ServiceCreation ServiceCreation::createExecuteByDataCmd ( const char* plugin_id, const char* id ) {
    ServiceCreation sc;
    sc.m_map[QLatin1String ( "id" ) ] = QString ( QLatin1String ( plugin_id ) + QLatin1String ( "_" ) + QLatin1String ( id ) );
    sc.m_map[QLatin1String ( "__type" ) ] = QLatin1String ( "execute" );
    return sc;
}

void ServiceCreation::setData ( const char* index, const QVariant& data ) {
    m_map[QLatin1String ( index ) ] = data;
}

QVariantMap ServiceCreation::getData() {
    return m_map;
}
