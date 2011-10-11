#include "servicecontroller.h"
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>
#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "plugincontroller.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#define __FUNCTION__ __FUNCTION__

ServiceController::ServiceController () : m_plugincontroller ( 0 ), m_last_changes_seq_nr ( 0 ) {
}

ServiceController::~ServiceController() {
    delete m_manager;
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
        qDebug() << "Response error:" << r->url();
        r->deleteLater();
        return;
    }

    if ( m_eventreplies.remove ( r ) ) {
        QByteArray line = r->readLine();
        if ( line.isEmpty() ) return;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if (ok) registerEvent ( data );
    } else if ( m_executecollection.remove ( r ) ) {
        bool ok;
        QVariantMap data = QJson::Parser().parse ( r->readAll(), &ok ).toMap();
        if ( ok && data.contains ( QLatin1String ( "rows" ) ) ) {
            QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
            for ( int i=0;i<list.size();++i ) {
                data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
                execute_action ( data );
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
    } else
        qDebug() << "received" << m_last_changes_seq_nr << r->url();

    r->deleteLater();
}

void ServiceController::replyEventsChange() {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    while ( r->canReadLine() ) {
        QByteArray line = r->readLine();
        if ( line.isEmpty() ) continue;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if ( ok && data.contains ( QLatin1String ( "seq" ) ) ) {
            int seq = data.value ( QLatin1String ( "seq" ) ).toInt();
            if ( seq > m_last_changes_seq_nr )
                m_last_changes_seq_nr = seq;
            if ( data.contains ( QLatin1String ( "deleted" ) ) ) {
                QPair<QVariantMap,AbstractPlugin_services*> d = m_registeredevents.value ( ServiceID::id ( data ) );
                AbstractPlugin_services* executeplugin = d.second;
                qDebug() << "deleted" << ServiceID::id ( data ) << d << executeplugin << data;
                if ( executeplugin )
                    executeplugin->unregister_event ( data, ServiceID::collectionid ( d.first ) );
				m_registeredevents.remove(ServiceID::id ( data ));
            } else {
                QNetworkRequest request ( QUrl ( QString ( QLatin1String ( "http://localhost:5984/roomcontrol/%1" ) ).arg ( data[QLatin1String ( "id" ) ].toString() ) ) );
                m_eventreplies.insert ( m_manager->get ( request ) );
            }
        }

    }
    if ( r->error() != QNetworkReply::NoError || !r->isRunning() ) {
        r->deleteLater();
        qDebug() << "Notification connection lost!";
    }
}

void ServiceController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void ServiceController::event_triggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    Q_UNUSED ( event_id );

    // request actions
    QNetworkRequest request ( QUrl ( QString ( QLatin1String ( "http://localhost:5984/roomcontrol/_design/app/_view/actions?key=\"%1\"" ) ).arg ( destination_collectionuid ) ) );

    QNetworkReply* r = m_manager->get ( request );
    m_executecollection.insert ( r );

// 	qDebug() << "event triggered" << event_id << r->url();

}

void ServiceController::execute_action ( const QVariantMap& data, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    if ( !ServiceID::isExecutable ( data ) && !ServiceID::isAction ( data ) ) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }

    executeplugin->execute ( data, QString() );
}

void ServiceController::property_changed ( const QVariantMap& data, const QString& sessionid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    qDebug() << "property changed" << data;
    //emit dataSync ( data, sessionid );
    QList<QString> plugins = m_propertyid_to_plugins.value ( ServiceID::id ( data ) ).toList();
    for ( int i=0;i<plugins.size();++i ) {
        AbstractPlugin_otherproperties* plugin = dynamic_cast<AbstractPlugin_otherproperties*> ( m_plugincontroller->getPlugin ( plugins[i] ) );
        if ( plugin ) plugin->otherPropertyChanged ( data, sessionid );
    }
}

void ServiceController::register_listener ( const QString& unqiue_property_id, const char* pluginid ) {
    m_propertyid_to_plugins[unqiue_property_id].insert ( QString::fromAscii ( pluginid ) );
}

void ServiceController::unregister_all_listeners ( const char* pluginid ) {
    const QString id = QString::fromAscii ( pluginid );
    QMutableMapIterator<QString, QSet<QString> > it ( m_propertyid_to_plugins );
    while ( it.hasNext() ) {
        it.next();
        it.value().remove ( id );
        if ( it.value().isEmpty() )
            it.remove();
    }
}

void ServiceController::unregister_listener ( const QString& unqiue_property_id, const char* pluginid ) {
    m_propertyid_to_plugins[unqiue_property_id].remove ( QString::fromAscii ( pluginid ) );
    if ( m_propertyid_to_plugins[unqiue_property_id].isEmpty() )
        m_propertyid_to_plugins.remove ( unqiue_property_id );
}
void ServiceController::requestEvents() {
    QNetworkRequest request ( QUrl ( QString ( QLatin1String ( "http://localhost:5984/roomcontrol/_design/app/_view/events#events" ) ).arg ( m_last_changes_seq_nr ) ) );
    m_manager->get ( request );
}

void ServiceController::startChangeLister() {
    QNetworkRequest request ( QUrl ( QString ( QLatin1String ( "http://localhost:5984/roomcontrol/_changes?feed=continuous&since=%1&filter=app/events&heartbeat=5000" ) ).arg ( m_last_changes_seq_nr ) ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = m_manager->get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange() ) );
}

void ServiceController::requestDatabaseInfo() {
    QNetworkRequest request ( QUrl ( QLatin1String ( "http://localhost:5984/roomcontrol#databaseinfo" ) ) );
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
            executeplugin->unregister_event ( data, destination_collectionuid );
            executeplugin->register_event ( data, destination_collectionuid );
            m_registeredevents.insert ( ServiceID::id ( data ),QPair<QVariantMap,AbstractPlugin_services*> ( data, executeplugin ) );
        }
    }
}
