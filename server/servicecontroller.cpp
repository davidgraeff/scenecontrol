#include "servicecontroller.h"

#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>

#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "config.h"
#include "plugincontroller.h"
#include "websocket.h"

#include <QSettings>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSocketNotifier>
#define __FUNCTION__ __FUNCTION__

ServiceController::ServiceController () : m_plugincontroller ( 0 ), m_last_changes_seq_nr ( 0 ) {
    m_websocket = new WebSocket();
    connect(m_websocket, SIGNAL(requestExecution(QVariantMap,int)),SLOT(requestExecution(QVariantMap,int)));
}

ServiceController::~ServiceController() {
    delete m_manager;
    delete m_websocket;
}

bool ServiceController::startWatchingCouchDB() {
    m_manager = new QNetworkAccessManager ( this );
    connect ( m_manager, SIGNAL ( finished ( QNetworkReply* ) ),
              this, SLOT ( networkReply ( QNetworkReply* ) ) );
    requestDatabaseInfo();

    return true;
}

void ServiceController::networkReply ( QNetworkReply* r ) {
    if ( r->error() != QNetworkReply::NoError ) {
        qWarning() << "Response error:" << r->url();
        r->deleteLater();
        return;
    }

    if ( m_eventreplies.remove ( r ) ) {
        QByteArray line = r->readLine();
        if ( line.isEmpty() ) return;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if ( ok ) registerEvent ( data );
    } else if ( m_executecollection.remove ( r ) ) {
        bool ok;
        QVariantMap data = QJson::Parser().parse ( r->readAll(), &ok ).toMap();
        if ( ok && data.contains ( QLatin1String ( "rows" ) ) ) {
            QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
            for ( int i=0;i<list.size();++i ) {
                data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
                pluginRequestExecution ( data );
            }
        }
    } else if ( r->url().fragment() ==QLatin1String ( "databaseinfo" ) ) {
        bool ok;
        QVariantMap data = QJson::Parser().parse ( r->readAll(), &ok ).toMap();
        if ( !data.contains ( QLatin1String ( "db_name" ) ) ) {

        } else {
            m_last_changes_seq_nr = data.value ( QLatin1String ( "update_seq" ),0 ).toInt();
            requestEvents();
        }
    } else if ( r->url().fragment() ==QLatin1String ( "events" ) ) {
        bool ok;
        QVariantMap data = QJson::Parser().parse ( r->readAll(), &ok ).toMap();
        if ( ok && data.contains ( QLatin1String ( "rows" ) ) ) {
            QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
            for ( int i=0;i<list.size();++i ) {
                data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
                registerEvent ( data );
            }
        }
        startChangeLister();
    } else if ( r->url().fragment() ==QLatin1String ( "changes" ) ) {
        startChangeLister();
    } else
        qDebug() << "received" << m_last_changes_seq_nr << r->url();

    r->deleteLater();
}

void ServiceController::replyEventsChange() {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    while ( r->canReadLine() ) {
        const QByteArray line = r->readLine();
        if ( line.size() <= 1 ) continue;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if ( ok && data.contains ( QLatin1String ( "seq" ) ) ) {
            const int seq = data.value ( QLatin1String ( "seq" ) ).toInt();
            if ( seq > m_last_changes_seq_nr )
                m_last_changes_seq_nr = seq;
            if ( data.contains ( QLatin1String ( "deleted" ) ) ) {
                const QString id = ServiceID::idChangeSeq ( data );
                QPair<QVariantMap,AbstractPlugin_services*> d = m_registeredevents.value ( id );
                AbstractPlugin_services* executeplugin = d.second;
                if ( executeplugin ) {
                    qDebug() << "unregister event" << id << executeplugin;
                    executeplugin->unregister_event ( d.first, ServiceID::collectionid ( d.first ), -1 );
                    m_registeredevents.remove ( id );
                }
            } else {
                const QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/%1" ).arg ( data[QLatin1String ( "id" ) ].toString() ) );
                m_eventreplies.insert ( m_manager->get ( request ) );
            }
        }

    }
    if ( r->error() != QNetworkReply::NoError) {
        r->deleteLater();
        qWarning() << "Notification connection lost!";
    }
}

void ServiceController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void ServiceController::pluginEventTriggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    Q_UNUSED ( event_id );

    // request actions
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/_design/roomcontrol/_view/actions?key=\"%1\"" ).arg ( destination_collectionuid ) );

    QNetworkReply* r = m_manager->get ( request );
    m_executecollection.insert ( r );
}

void ServiceController::requestExecution(const QVariantMap& data, int sessionid) {
    if ( !ServiceID::isExecutable ( data ) && !ServiceID::isAction ( data ) ) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }

    executeplugin->execute ( data, sessionid );
}

void ServiceController::pluginRequestExecution ( const QVariantMap& data, const char* pluginid ) {
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

void ServiceController::pluginPropertyChanged ( const QVariantMap& data, int sessionid, const char* pluginid ) {
    Q_UNUSED ( pluginid );

    QList<QString> plugins = m_propertyid_to_plugins.value ( ServiceID::id ( data ) ).toList();
    for ( int i=0;i<plugins.size();++i ) {
        AbstractPlugin_otherproperties* plugin = dynamic_cast<AbstractPlugin_otherproperties*> ( m_plugincontroller->getPlugin ( plugins[i] ) );
        if ( plugin ) plugin->otherPropertyChanged ( data, sessionid );
    }

    //emit dataSync ( data, sessionid );
    QByteArray jsondata = QJson::Serializer().serialize(data);
    if (!jsondata.isEmpty())
        m_websocket->sendToAllClients(jsondata);
}

QByteArray ServiceController::allProperties(int sessionid) {
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

void ServiceController::pluginRegisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid ) {
    m_propertyid_to_plugins[unqiue_property_id].insert ( QString::fromAscii ( pluginid ) );
}

void ServiceController::pluginUnregisterAllPropertyChangeListeners ( const char* pluginid ) {
    const QString id = QString::fromAscii ( pluginid );
    QMutableMapIterator<QString, QSet<QString> > it ( m_propertyid_to_plugins );
    while ( it.hasNext() ) {
        it.next();
        it.value().remove ( id );
        if ( it.value().isEmpty() )
            it.remove();
    }
}

void ServiceController::pluginUnregisterPropertyChangeListener ( const QString& unqiue_property_id, const char* pluginid ) {
    m_propertyid_to_plugins[unqiue_property_id].remove ( QString::fromAscii ( pluginid ) );
    if ( m_propertyid_to_plugins[unqiue_property_id].isEmpty() )
        m_propertyid_to_plugins.remove ( unqiue_property_id );
}
void ServiceController::requestEvents() {
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/_design/roomcontrol/_view/events#events" ) );
    m_manager->get ( request );
}

void ServiceController::startChangeLister() {
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/_changes?feed=continuous&since=%1&filter=roomcontrol/events&heartbeat=5000#changes" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = m_manager->get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange() ) );
}

void ServiceController::requestDatabaseInfo() {
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol#databaseinfo" ) );
    m_manager->get ( request );
}
void ServiceController::registerEvent ( const QVariantMap& data ) {
    if ( data.contains ( QLatin1String ( "_id" ) ) ) {
        if ( !data.contains ( QLatin1String ( "collection_" ) ) ) {
            qWarning() <<"Cannot register event. No collection set:"<<ServiceID::pluginid ( data ) << ServiceID::id ( data );
            return;
        }
        AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
        AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
        if ( !executeplugin ) {
            qWarning() <<"Cannot register event. No plugin found:"<<ServiceID::pluginid ( data ) << ServiceID::id ( data );
            return;
        }
        qDebug() << "register event:" << ServiceID::id ( data ) << ServiceID::pluginid ( data ) << ServiceID::pluginmember ( data );
        QString destination_collectionuid = ServiceID::collectionid ( data );
        if ( destination_collectionuid.size() ) {
            executeplugin->unregister_event ( data, destination_collectionuid, -1 );
            executeplugin->register_event ( data, destination_collectionuid, -1 );
            m_registeredevents.insert ( ServiceID::id ( data ),QPair<QVariantMap,AbstractPlugin_services*> ( data, executeplugin ) );
        }
    }
}
