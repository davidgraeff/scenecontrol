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

ServiceController::ServiceController () : m_plugincontroller ( 0 ), m_last_changes_seq_nr(0) {
}

ServiceController::~ServiceController() {
	delete m_manager;
}

bool ServiceController::startWatchingCouchDB()
{
	m_manager = new QNetworkAccessManager(this);
	connect(m_manager, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(networkReply(QNetworkReply*)));

	QNetworkRequest request(QUrl(QString(QLatin1String("http://localhost:5984/roomcontrol/_changes?feed=continuous&since=%1&filter=app/events&heartbeat=5000")).arg(m_last_changes_seq_nr)));
	request.setRawHeader("Connection","keep-alive");
	QNetworkReply *r = m_manager->get(request);
	connect(r, SIGNAL(readyRead()), SLOT(replyEventsChange()));
	return true;
}

void ServiceController::networkReply(QNetworkReply* r)
{
	if (m_eventreplies.remove(r)) {
		QByteArray line = r->readLine();
		if (line.isEmpty()) return;
		bool ok;
		QVariantMap data = QJson::Parser().parse(line, &ok).toMap();
		if (ok && data.contains(QLatin1String("_id"))) {
			if (!data.contains(QLatin1String("collection_"))) {
				qWarning() <<"Cannot register event. No collection set:"<<ServiceID::pluginid ( data ) << ServiceID::id(data);
				return;
			}
			AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
			AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
			if ( !executeplugin ) {
				qWarning() <<"Cannot register event. No plugin found:"<<ServiceID::pluginid ( data ) << ServiceID::id(data);
				return;
			}
			qDebug() << "register event:" << data;
			executeplugin->register_event(data, ServiceID::collectionid(data));
			m_registeredevents.insert(ServiceID::id(data),executeplugin);
		}
	} else
	qDebug() << "received" << m_last_changes_seq_nr << r->url();
}

void ServiceController::replyEventsChange()
{
	QNetworkReply *r = (QNetworkReply*)sender();
	while (r->canReadLine()) {
		QByteArray line = r->readLine();
		if (line.isEmpty()) continue;
		bool ok;
		QVariantMap data = QJson::Parser().parse(line, &ok).toMap();
		if (ok && data.contains(QLatin1String("seq"))) {
			int seq = data.value(QLatin1String("seq")).toInt();
			if (seq > m_last_changes_seq_nr)
				m_last_changes_seq_nr = seq;
			if (data.contains(QLatin1String("deleted"))) {
				qDebug() << "deleted";
				AbstractPlugin_services* executeplugin = m_registeredevents.value(ServiceID::id(data));
				if (executeplugin)
					executeplugin->unregister_event(data, ServiceID::collectionid(data));
			} else {
				QNetworkRequest request(QUrl(QString(QLatin1String("http://localhost:5984/roomcontrol/%1")).arg(data[QLatin1String("id")].toString())));
				m_eventreplies.insert(m_manager->get(request));
			}
		}
		
	}
	if (r->error() != QNetworkReply::NoError || !r->isRunning()) {
		r->deleteLater();
		qDebug() << "Notification connection lost!";
	}
}

void ServiceController::checkConditions(const QString& collectionid)
{

}

void ServiceController::executeActions(const QString& collectionid)
{

}

void ServiceController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void ServiceController::event_triggered ( const QString& event_id, const QString& destination_collectionuid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    Q_UNUSED ( event_id );
    
	QNetworkRequest request(QUrl(QString(QLatin1String("http://localhost:5984/roomcontrol/%1")).arg(data[QLatin1String("id")].toString())));
	m_eventreplies.insert(m_manager->get(request));
    
    Q_UNUSED ( destination_collectionuid );
}

void ServiceController::execute_action ( const QVariantMap& data, const char* pluginid ) {
    if ( !ServiceID::isExecutable ( data ) ) return;
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
