#include "couchdb.h"

#include <QSettings>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSocketNotifier>
#include <qeventloop.h>
#include <QTimer>
#include <QDir>
#include <json.h>
#include "../../../shared/pluginservicehelper.h"

#define __FUNCTION__ __FUNCTION__

static Database* databaseInstance = 0;


Database::CouchDB () : m_last_changes_seq_nr ( 0 ), m_settingsChangeFailCounter(0), m_eventsChangeFailCounter(0) {
}

Database::~CouchDB() {
}


QString Database::couchdbAbsoluteUrl(const QString& relativeUrl) {
    return m_serveraddress + QLatin1String("/") + relativeUrl;
}

Database* Database::instance()
{
    if (databaseInstance == 0)
        databaseInstance = new Database();

    return databaseInstance;
}

bool Database::connectToDatabase(const QString &couchdburl) {
    m_serveraddress = couchdburl;
    QEventLoop eventLoop;
    QNetworkReply *r;

    { // Connect to database (try to create it if not available)
        qDebug() << "CouchDB:" << couchdbAbsoluteUrl();
        QNetworkRequest request ( couchdbAbsoluteUrl() );
        r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if (r->error() == QNetworkReply::ContentNotFoundError) {
            qWarning() << "CouchDB: Network error" << r->error();
            return false;
        }
    }

    { // try to parse database information
        QByteArray rawdata = r->readAll();
        QVariantMap data = JSON::parse ( rawdata ).toMap();
        if (data.isEmpty()) {
            // Response is not json: no error recovery possible
            qWarning() << "CouchDB: Json parser:" << rawdata;
            return false;
        }

        if ( !data.contains ( QLatin1String ( "db_name" ) ) || !data.contains ( QLatin1String ( "doc_count" ) ) ) {
            // Response is not expected without db_name: no error recovery possible
            qWarning() << "CouchDB: db_name or doc_count not found";
            return false;
        }

        int doccount = data.value(QLatin1String("doc_count")).toInt();
        if (!doccount) {
            // Initial data could not be installed
            qWarning() << "CouchDB: doc_count = 0";
            return false;
        }
        m_last_changes_seq_nr = data.value ( QLatin1String ( "update_seq" ),0 ).toInt();
    }

    emit ready();
    return true;
}

void Database::requestEvents(const QString& plugin_id)
{
    QEventLoop eventLoop;
    QNetworkRequest request ( couchdbAbsoluteUrl("_design/_server/_view/events?key=\"%1\"#%1" ).arg ( plugin_id ) );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();
    if (r->error() != QNetworkReply::NoError) {
        // Database events could not be read: no error recovery possible
        qWarning() << "CouchDB: " << r->readAll() << request.url();
        return;
    }

    QByteArray rawdata = r->readAll();
    QVariantMap data = JSON::parse ( rawdata ).toMap();
    if (data.isEmpty()) {
        // Response is not json: no error recovery possible
        qWarning() << "CouchDB: Json parser:" << rawdata;
        return;
    }
    if (data.contains ( QLatin1String ( "rows" ) )) {
        QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        for ( int i=0;i<list.size();++i ) {
            data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
            data.remove(QLatin1String("_rev"));
            data.remove(QLatin1String("type_"));
            if ( ServiceData::collectionid ( data ).isEmpty() ) {
                qWarning() <<"CouchDB: Received event without collection:"<<ServiceData::pluginid ( data ) << ServiceData::id ( data );
                continue;
            }
            emit Event_add ( ServiceData::id(data), data );
        }
    }
}


void Database::replyEventsChange() {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    if ( r->error() != QNetworkReply::NoError ) {
        ++m_eventsChangeFailCounter;
        if (m_eventsChangeFailCounter > 5) {
            qWarning() << "CouchDB: replyEventsChange" << r->url();
        } else {
            QTimer::singleShot(1000, this, SLOT(startChangeListenerEvents()));
        }
        delete r;
        return;
    }

    m_eventsChangeFailCounter = 0;

    while ( r->canReadLine() ) {
        QString docid;
        { // one line = one changed event; evaluate and extract docid
            const QByteArray line = r->readLine();
            if ( line.size() <= 1 ) continue;
            QVariantMap data = JSON::parse ( line ).toMap();
            if ( data.isEmpty() || !data.contains ( QLatin1String ( "seq" ) ) )
                continue;
            const int seq = data.value ( QLatin1String ( "seq" ) ).toInt();
            if ( seq > m_last_changes_seq_nr )
                m_last_changes_seq_nr = seq;

            if ( data.contains ( QLatin1String ( "deleted" ) ) ) {
                emit Event_remove(ServiceData::idChangeSeq ( data ));
                continue;
            }


            docid = ServiceData::idChangeSeq ( data );
        }

        // request document that is mentioned in the changes feed
        QNetworkRequest request ( couchdbAbsoluteUrl(docid));

        QNetworkReply* r = get ( request );
        QEventLoop eventLoop;
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();

        QVariantMap document = JSON::parse(r->readAll() ).toMap();

        if (!ServiceData::checkType(document, ServiceData::TypeEvent)) {
            qWarning() << "Event changed but is not of type event" << document;
            continue;
        }
        document.remove(QLatin1String("_rev"));
        document.remove(QLatin1String("type_"));
        if ( ServiceData::collectionid ( document ).isEmpty() ) {
            qWarning() <<"CouchDB: Received event without collection:"<<ServiceData::pluginid ( document ) << ServiceData::id ( document );
            continue;
        }
        emit Event_add ( ServiceData::id(document), document );
    }
}

void Database::startChangeListenerEvents()
{
    QNetworkRequest request ( couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/events&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange()) );
    //connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void Database::startChangeListenerSettings()
{
    QNetworkRequest request ( couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/settings&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyPluginSettingsChange() ) );
    //connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

bool Database::checkFailure(QNetworkReply* r)
{
    if ( r->error() != QNetworkReply::NoError ) {
        qWarning() << "CouchDB: Response error:" << r->url();
        emit failed(r->url().toString());
        return true;
    }
    return false;
}

void Database::errorWithRecovery(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "CouchDB: Network error:" << e << r->url().toString();
}

void Database::errorFatal(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "CouchDB: Fatal Network error:" << e << r->url().toString();
    emit failed(r->url().toString());
}

void Database::replyDataOfCollection()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    if (checkFailure(r))
        return;

    QVariantMap data = JSON::parse( r->readAll() ).toMap();
    if ( !data.isEmpty() && data.contains ( QLatin1String ( "rows" ) ) ) {
        QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        QList<QVariantMap> actionsList;
        QList<QVariantMap> conditionList;
        for (int i=0;i<list.size();++i) {
            QVariantMap data = list.value(i).toMap().value ( QLatin1String ( "value" ) ).toMap();
            // Check if type is action type
            if ( ServiceData::checkType ( data, ServiceData::TypeCondition ))
                conditionList.append(data);
            else if ( ServiceData::checkType ( data, ServiceData::TypeAction ))
                actionsList.append(data);
        }
        emit dataOfCollection( actionsList, conditionList, r->url().fragment() );
    }
}

void Database::requestDataOfCollection(const QString& collecion_id)
{
    if (collecion_id.isEmpty())
        return;
    QNetworkRequest request ( couchdbAbsoluteUrl("_design/_server/_view/actionsconditions?key=\"%1\"#%1" ).arg ( collecion_id ) );

    QNetworkReply* r = get ( request );
    connect ( r, SIGNAL ( finished() ), SLOT ( replyDataOfCollection()) );
    connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void Database::requestPluginSettings(const QString& pluginid, bool tryToInstall)
{
    if (pluginid.isEmpty())
        return;
    QNetworkRequest request ( couchdbAbsoluteUrl("_design/_server/_view/settings?key=\"%1\"" ).arg(pluginid));

    QNetworkReply* r = get ( request );
    QEventLoop eventLoop;
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();

    if ( r->error() != QNetworkReply::NoError) {
        qWarning()<<"CouchDB: Get settings failed for" << pluginid;

    }

    QVariantMap data = JSON::parse ( r->readAll() ).toMap();
    if ( !data.isEmpty() && data.contains ( QLatin1String ( "rows" ) ) ) {
        QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        if (list.isEmpty()) {
                qWarning()<<"CouchDB: Get settings failed for" << pluginid;
                return;
        }
        foreach(const QVariant& v, list) {
            data = v.toMap().value(QLatin1String ( "value" )).toMap();
            data.remove(QLatin1String("_id"));
            data.remove(QLatin1String("_rev"));
            data.remove(QLatin1String("type_"));
            data.remove(QLatin1String("plugin_"));
            const QString key = data.take(QLatin1String("key_")).toString();
            if (data.size())
                emit settings ( pluginid, key, data );
        }
    } else {
        qWarning() << "CouchDB: Get settings failed for" << pluginid << data;
    }

    delete r;
}

void Database::replyPluginSettingsChange()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    if ( r->error() != QNetworkReply::NoError ) {
        ++m_settingsChangeFailCounter;
        if (m_settingsChangeFailCounter > 5) {
            qWarning() << "CouchDB: replyPluginSettingsChange" << r->url();
        } else {
            QTimer::singleShot(1000, this, SLOT(replyPluginSettingsChange()));
        }
        delete r;
        return;
    }

    m_settingsChangeFailCounter = 0;

    while ( r->canReadLine() ) {
        const QByteArray line = r->readLine();
        if ( line.size() <= 1 ) continue;
        QVariantMap data = JSON::parse(line ).toMap();
        if ( !data.isEmpty() || !data.contains ( QLatin1String ( "seq" ) ) )
            continue;

        const int seq = data.value ( QLatin1String ( "seq" ) ).toInt();

        if ( seq > m_last_changes_seq_nr )
            m_last_changes_seq_nr = seq;

        if ( data.contains ( QLatin1String ( "deleted" ) ) )
            continue;

        const QString docid = ServiceData::idChangeSeq ( data );

        // request document that is mentioned in the changes feed
        QNetworkRequest request ( couchdbAbsoluteUrl(docid));

        QNetworkReply* r = get ( request );
        QEventLoop eventLoop;
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();

        QVariantMap document = JSON::parse(r->readAll() ).toMap();

        if (!ServiceData::checkType(document, ServiceData::TypeConfiguration)) {
            qWarning() << "Configuration changed but is not of type configuration" << document;
            continue;
        }

        document.remove(QLatin1String("_id"));
        document.remove(QLatin1String("_rev"));
        document.remove(QLatin1String("type_"));
        document.remove(QLatin1String("plugin_"));
        const QString key = data.take(QLatin1String("key_")).toString();
        emit settings ( ServiceData::pluginid(document), key, document );
    }
}

void Database::errorNoSettings()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    emit couchDB_no_settings_found(r->url().fragment());
}

