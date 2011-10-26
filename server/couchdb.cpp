#include "couchdb.h"

#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "config.h"

#include <QSettings>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSocketNotifier>
#include <shared/abstractplugin.h>
#define __FUNCTION__ __FUNCTION__

static CouchDB* couchdbInstance = 0;

CouchDB::CouchDB () : m_last_changes_seq_nr ( 0 ) {
}

CouchDB::~CouchDB() {
}

CouchDB* CouchDB::instance()
{
    if (couchdbInstance == 0)
        couchdbInstance = new CouchDB();

    return couchdbInstance;
}

void CouchDB::start() {
    qDebug() << "CouchDB:" << setup::couchdbAbsoluteUrl("" );
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("" ) );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL(finished()), SLOT ( replyDatabaseInfo() ) );
    connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)) );
}

bool CouchDB::checkFailure(QNetworkReply* r)
{
    if ( r->error() != QNetworkReply::NoError ) {
        qWarning() << "Response error:" << r->url();
        emit couchDB_failed(r->url().toString());
        return true;
    }
    return false;
}

void CouchDB::replyDatabaseInfo()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    if (checkFailure(r))
        return;

    bool ok;
    QByteArray rawdata = r->readAll();
    QVariantMap data = QJson::Parser().parse ( rawdata, &ok ).toMap();
    if (!ok) {
        qWarning() << "Json parser:" << rawdata;
        return;
    }
    if ( !data.contains ( QLatin1String ( "db_name" ) ) ) {
        qWarning() << "Connection to couchdb failed!";
        emit couchDB_failed(r->url().toString());
    } else {
        m_last_changes_seq_nr = data.value ( QLatin1String ( "update_seq" ),0 ).toInt();
        emit couchDB_ready();
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/roomcontrol/_view/events" ) );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( finished() ), SLOT ( replyEvents() ) );
        connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)) );
    }
}

void CouchDB::replyEvent()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    if (checkFailure(r))
        return;

    while (r->canReadLine()) {
        QByteArray line = r->readLine();
        if ( line.isEmpty() ) return;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if ( ok )
            emit couchDB_Event_add ( ServiceID::id(data), data );
        else {
            qWarning() << "Json parser:" << line;
            return;
        }
    }
}

void CouchDB::replyEvents()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    if (checkFailure(r))
        return;

    bool ok;
    QByteArray rawdata = r->readAll();
    QVariantMap data = QJson::Parser().parse ( rawdata, &ok ).toMap();
    if (!ok) {
        qWarning() << "Json parser:" << rawdata;
        return;
    }
    if ( ok && data.contains ( QLatin1String ( "rows" ) ) ) {
        QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        for ( int i=0;i<list.size();++i ) {
            data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
            emit couchDB_Event_add ( ServiceID::id(data), data );
        }
    }

    // startChangeLister
    {
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=roomcontrol/events&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
        request.setRawHeader ( "Connection","keep-alive" );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange() ) );
        connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)) );
    }
}

void CouchDB::replyActionOfCollection()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    if (checkFailure(r))
        return;

    bool ok;
    QVariantMap data = QJson::Parser().parse ( r->readAll(), &ok ).toMap();
    if ( ok && data.contains ( QLatin1String ( "rows" ) ) ) {
        const QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        emit couchDB_actionsOfCollection( list, r->url().fragment() );
    }
}

void CouchDB::replyEventsChange() {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    if (checkFailure(r))
        return;

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
                emit couchDB_Event_remove(ServiceID::idChangeSeq ( data ));
            } else {
                const QNetworkRequest request ( setup::couchdbAbsoluteUrl("%1" ).arg ( data[QLatin1String ( "id" ) ].toString() ) );
                QNetworkReply* r = get ( request );
                connect ( r, SIGNAL ( finished() ), SLOT ( replyEvent() ) );
                connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)) );
            }
        }

    }
}

void CouchDB::requestActionsOfCollection(const QString& collecion_id)
{
  if (collecion_id.isEmpty())
    return;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/roomcontrol/_view/actions?key=\"%1\"#%1" ).arg ( collecion_id ) );

    QNetworkReply* r = get ( request );
    connect ( r, SIGNAL ( finished() ), SLOT ( replyActionOfCollection() ) );
    connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)) );
}
void CouchDB::error(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "CouchDB: Network error:" << e << r->url().toString();
}

