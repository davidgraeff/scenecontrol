#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"

#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "config.h"
#include "plugincontroller.h"
#include "websocket.h"
#include <QDebug>
#define __FUNCTION__ __FUNCTION__

#include "collectioncontroller.h"
#include "couchdb.h"

CollectionController::CollectionController () : m_plugincontroller ( 0 ) {}

CollectionController::~CollectionController() {}

void CollectionController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void CollectionController::pluginEventTriggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    Q_UNUSED ( event_id );
    CouchDB::instance()->requestActionsOfCollection(destination_collectionuid);
}

void CollectionController::requestExecution(const QVariantMap& data, int sessionid) {
    if ( !ServiceID::isExecutable ( data ) || sessionid == -1) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }
qDebug() << "Execute" << data;
    executeplugin->execute ( data, sessionid );
}

void CollectionController::actionsOfCollection(const QVariantList& actions, const QString& collectionid)
{
    Q_UNUSED(collectionid);
    for ( int i=0;i<actions.size();++i ) {
        QVariantMap actiondata = actions.value(i).toMap().value ( QLatin1String ( "value" ) ).toMap();
        AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( actiondata ) );
        AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
        if ( !executeplugin ) {
            qWarning() <<"Cannot execute service. No plugin found:"<<actiondata;
            return;
        }

        executeplugin->execute ( actiondata, -1 );
    }
}

void CollectionController::pluginRequestExecution ( const QVariantMap& data, const char* pluginid ) {
    if ( !ServiceID::isExecutable ( data ) && !ServiceID::isAction ( data ) ) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }
    if (pluginid)
        qDebug() << "Plugin" << pluginid << "executes action";

    executeplugin->execute ( data, -1 );
}

void CollectionController::execute(const QVariantMap& data, int sessionid) {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "executecollection" ) ) {
        CouchDB::instance()->requestActionsOfCollection(data.value(QLatin1String("collectionid")).toString());
    }
}
