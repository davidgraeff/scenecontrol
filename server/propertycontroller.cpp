#include "propertycontroller.h"

#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractplugin_otherproperties.h>

#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "config.h"
#include "plugincontroller.h"
#include "websocket.h"
#include <QDebug>
#define __FUNCTION__ __FUNCTION__

PropertyController::PropertyController () : m_plugincontroller ( 0 ) {}

PropertyController::~PropertyController() {
}


void PropertyController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void PropertyController::pluginPropertyChanged ( const QVariantMap& data, int sessionid, const char* pluginid ) {
    Q_UNUSED ( pluginid );

    QList<QString> plugins = m_propertyid_to_plugins.value ( ServiceID::id ( data ) ).toList();
    for ( int i=0;i<plugins.size();++i ) {
        AbstractPlugin_otherproperties* plugin = dynamic_cast<AbstractPlugin_otherproperties*> ( m_plugincontroller->getPlugin ( plugins[i] ) );
        if ( plugin ) plugin->otherPropertyChanged ( data, sessionid );
    }

    //emit dataSync ( data, sessionid );
    QByteArray jsondata = QJson::Serializer().serialize(data);
    if (!jsondata.isEmpty()) {
        if (sessionid == -1)
            WebSocket::instance()->sendToAllClients(jsondata);
        else
            WebSocket::instance()->sendToClient(jsondata, sessionid);
    } else if (data.size()) {
	qWarning() << "Json Serializer failed at:" << data;
    }
}

QByteArray PropertyController::allProperties(int sessionid) {
    Q_ASSERT(m_plugincontroller);
    QByteArray jsondata;
    QMap<QString,PluginInfo*>::iterator i = m_plugincontroller->getPluginIterator();
    while (AbstractPlugin_services* plugin = m_plugincontroller->nextServicePlugin(i)) {
        QList<QVariantMap> properties = plugin->properties ( sessionid );
        for (int i=0;i<properties.size();++i)
            jsondata += QJson::Serializer().serialize(properties[i]) + "\n";
    }
    return jsondata;
}

void PropertyController::pluginRegisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid ) {
    m_propertyid_to_plugins[unqiue_property_id].insert ( QString::fromAscii ( pluginid ) );
}

void PropertyController::pluginUnregisterAllPropertyChangeListeners ( const char* pluginid ) {
    const QString id = QString::fromAscii ( pluginid );
    QMutableMapIterator<QString, QSet<QString> > it ( m_propertyid_to_plugins );
    while ( it.hasNext() ) {
        it.next();
        it.value().remove ( id );
        if ( it.value().isEmpty() )
            it.remove();
    }
}

void PropertyController::pluginUnregisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid ) {
    m_propertyid_to_plugins[unqiue_property_id].remove ( QString::fromAscii ( pluginid ) );
    if ( m_propertyid_to_plugins[unqiue_property_id].isEmpty() )
        m_propertyid_to_plugins.remove ( unqiue_property_id );
}

void PropertyController::execute(const QVariantMap& data, int sessionid) {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "requestProperties" ) ) {
        WebSocket::instance()->sendToClient(allProperties(sessionid), sessionid);
    }
}
