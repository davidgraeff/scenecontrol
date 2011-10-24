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
#include "libwebsocket/libwebsockets.h"
#include "paths.h"
#include "config.h"
#include "plugincontroller.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSocketNotifier>
#include "libwebsocket/private-libwebsockets.h"

#define __FUNCTION__ __FUNCTION__

//////////////////// libwebsocket ///////////////////////

static ServiceController* servicecontroller;

enum libwebsocket_protocols_enum {
	/* always first */
	PROTOCOL_HTTP = 0,

	PROTOCOL_ROOMCONTROL,

	/* always last */
	PROTOCOL_COUNT
};


static int callback_http(struct libwebsocket_context * context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
  Q_UNUSED(context);
  Q_UNUSED(wsi);
  Q_UNUSED(reason);
  Q_UNUSED(user);
  Q_UNUSED(in);
  Q_UNUSED(len);
  
	switch (reason) {
	case LWS_CALLBACK_ADD_POLL_FD:
// 		pollfds[count_pollfds].fd = (int)(long)user;
// 		pollfds[count_pollfds].events = (int)len;
// 		pollfds[count_pollfds++].revents = 0;
		servicecontroller->addWebsocketFD((int)(long)user, (int)len);
		break;

	case LWS_CALLBACK_DEL_POLL_FD:
	  servicecontroller->removeWebsocketFD((int)(long)user);
// 		for (n = 0; n < count_pollfds; n++)
// 			if (pollfds[n].fd == (int)(long)user)
// 				while (n < count_pollfds) {
// 					pollfds[n] = pollfds[n + 1];
// 					n++;
// 				}
// 		count_pollfds--;

		break;

	case LWS_CALLBACK_SET_MODE_POLL_FD:
// 		for (n = 0; n < count_pollfds; n++)
// 			if (pollfds[n].fd == (int)(long)user)
// 				pollfds[n].events |= (int)(long)len;
		break;

	case LWS_CALLBACK_CLEAR_MODE_POLL_FD:
// 		for (n = 0; n < count_pollfds; n++)
// 			if (pollfds[n].fd == (int)(long)user)
// 				pollfds[n].events &= ~(int)(long)len;
		break;

	default:
		break;
	}

	return 0;
}


static int callback_roomcontrol_protocol(struct libwebsocket_context * context,
			struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
  Q_UNUSED(context);
  Q_UNUSED(wsi);
  Q_UNUSED(user);
  int n;
	switch (reason) {
	case LWS_CALLBACK_BROADCAST:
		n = libwebsocket_write(wsi, &((unsigned char*)in)[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT);
		if (n < 0) {
			fprintf(stderr, "ERROR writing to socket");
			return 1;
		}
// 		if (close_testing && pss->number == 50) {
// 			fprintf(stderr, "close tesing limit, closing\n");
// 			libwebsocket_close_and_free_session(context, wsi,
// 						       LWS_CLOSE_STATUS_NORMAL);
// 		}
		break;
	case LWS_CALLBACK_RECEIVE:
		if (len < 3)
			break;
		if (strcmp((char*)in, "all\n") == 0)
			servicecontroller->websocketClientRequestAllProperties(wsi);
		break;
	default:
		break;
	}
	
	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"http-only",		/* name */
		callback_http,		/* callback */
		0,			/* per_session_data_size */
		0,0,0,0
	},
	{
		"roomcontrol-protocol",
		callback_roomcontrol_protocol,
		sizeof(void*), //aditional data: pointer to ServiceController
		0,0,0,0
	},
	{
		NULL, NULL, 0,		/* End of list */
		0,0,0,0
	}
};

//////////////////// ServiceController ///////////////////////

ServiceController::ServiceController () : m_plugincontroller ( 0 ), m_last_changes_seq_nr ( 0 ), m_websocket_context( 0 ) {
    servicecontroller = this; // remember this servicecontroller object in a global variable
    m_websocket_context = libwebsocket_create_context(ROOM_LISTENPORT, 0, protocols,
				libwebsocket_internal_extensions,
				certificateFile("server.crt").toLatin1().constData(), certificateFile("server.key").toLatin1().constData(), -1, -1, 0);
    
    if (m_websocket_context == 0) {
	    qWarning() << "libwebsocket init failed";
    } else {
      for (int i=0; i< m_websocket_context->fds_count; ++i) {
	    addWebsocketFD(m_websocket_context->fds[i].fd, m_websocket_context->fds[i].events);
      }
      qDebug() << "SSL Websocket ready on port" << ROOM_LISTENPORT;
    }
}

ServiceController::~ServiceController() {
    delete m_manager;
    libwebsocket_context_destroy(m_websocket_context);
    qDeleteAll(m_websocket_fds);
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
    } else if ( r->url().fragment() ==QLatin1String ( "changes" ) ) {
        startChangeLister();
    } else
        qDebug() << "received" << m_last_changes_seq_nr << r->url();

    r->deleteLater();
}

void ServiceController::replyEventsChange() {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    while ( r->canReadLine() ) {
        QByteArray line = r->readLine();
        if ( line.size() <= 1 ) continue;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if ( ok && data.contains ( QLatin1String ( "seq" ) ) ) {
            int seq = data.value ( QLatin1String ( "seq" ) ).toInt();
            if ( seq > m_last_changes_seq_nr )
                m_last_changes_seq_nr = seq;
            if ( data.contains ( QLatin1String ( "deleted" ) ) ) {
                QString id = ServiceID::idChangeSeq ( data );
                QPair<QVariantMap,AbstractPlugin_services*> d = m_registeredevents.value ( id );
                AbstractPlugin_services* executeplugin = d.second;
                qDebug() << "unregister event" << id << executeplugin;
                if ( executeplugin )
                    executeplugin->unregister_event ( d.first, ServiceID::collectionid ( d.first ) );
                m_registeredevents.remove ( id );
            } else {
                QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/%1" ).arg ( data[QLatin1String ( "id" ) ].toString() ) );
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

void ServiceController::event_triggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    Q_UNUSED ( event_id );

    // request actions
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/_design/app/_view/actions?key=\"%1\"" ).arg ( destination_collectionuid ) );

    QNetworkReply* r = m_manager->get ( request );
    m_executecollection.insert ( r );
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

    QList<QString> plugins = m_propertyid_to_plugins.value ( ServiceID::id ( data ) ).toList();
    for ( int i=0;i<plugins.size();++i ) {
        AbstractPlugin_otherproperties* plugin = dynamic_cast<AbstractPlugin_otherproperties*> ( m_plugincontroller->getPlugin ( plugins[i] ) );
        if ( plugin ) plugin->otherPropertyChanged ( data, sessionid );
    }
    
    //emit dataSync ( data, sessionid );
    if (m_websocket_context) {
      // send data over websocket. First convert to json string then copy to a c buffer and send to all connected websocket clients with the right protocol
      QByteArray jsondata = QJson::Serializer().serialize(data);
      const int len = jsondata.size();
      unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + len];
      memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING,jsondata.constData(),len);
      libwebsockets_broadcast(&protocols[PROTOCOL_ROOMCONTROL], buf, len);
    }
}

void ServiceController::websocketClientRequestAllProperties(libwebsocket* wsi) {
  Q_ASSERT(m_plugincontroller);
    QByteArray jsondata;
    QMap<QString,PluginInfo*>::iterator i = m_plugincontroller->getPluginIterator();
    while (AbstractPlugin_services* plugin = m_plugincontroller->nextServicePlugin(i)) {
       QList<QVariantMap> properties = plugin->properties ( QString() );
       for (int i=0;i<properties.size();++i)
	jsondata += QJson::Serializer().serialize(properties[i]) + "\n";
    }
    
    if (m_websocket_context) {
      unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + jsondata.size()];
      unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
      memcpy(p,jsondata.constData(),jsondata.size());
      int n = libwebsocket_write(wsi, p, jsondata.size(), LWS_WRITE_TEXT);
      if (n < 0) {
	      qWarning() << "ERROR writing to socket: websocketClientRequestAllProperties";
      }
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
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/_design/app/_view/events#events" ) );
    m_manager->get ( request );
}

void ServiceController::startChangeLister() {
    QNetworkRequest request ( couchdbAbsoluteUrl("roomcontrol/_changes?feed=continuous&since=%1&filter=app/events&heartbeat=5000#changes" ).arg ( m_last_changes_seq_nr ) );
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
            executeplugin->unregister_event ( data, destination_collectionuid );
            executeplugin->register_event ( data, destination_collectionuid );
            m_registeredevents.insert ( ServiceID::id ( data ),QPair<QVariantMap,AbstractPlugin_services*> ( data, executeplugin ) );
        }
    }
}

void ServiceController::websocketactivity(int) {
  libwebsocket_service(m_websocket_context,0);
}
void ServiceController::addWebsocketFD(int fd, short int direction) {
      if (m_websocket_fds.contains(fd))
	return;
      QSocketNotifier::Type _direction = ((direction == POLLOUT) ? QSocketNotifier::Write : QSocketNotifier::Read);
      QSocketNotifier* sn = new QSocketNotifier(fd, _direction, this);
      connect(sn, SIGNAL(activated(int)), SLOT(websocketactivity(int)));
      sn->setEnabled(true);
      m_websocket_fds.insert(fd, sn);
}

void ServiceController::removeWebsocketFD(int fd) {
  delete m_websocket_fds.take(fd);
}
