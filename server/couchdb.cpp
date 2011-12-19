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
#include <qeventloop.h>
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

bool CouchDB::connectToDatabase() {
    QEventLoop eventLoop;
    // Connect to database for basic information
    {
        qDebug() << "CouchDB:" << setup::couchdbAbsoluteUrl("" );
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("" ) );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if (r->error() == QNetworkReply::ContentNotFoundError) {
            r = put(request, "");
            connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            if (r->error() != QNetworkReply::NoError) {
                // Database could not be created: no error recovery possible
                qWarning() << "CouchDB: Database not found and could not be created!";
                return false;
            }
            r = get ( request );
            connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            if (r->error() != QNetworkReply::NoError) {
                // Database created but could not be read: no error recovery possible
                qWarning() << "CouchDB: Successfull created database but can not read it out!";
                return false;
            }
        } else if (r->error() != QNetworkReply::NoError) {
            // Network error: no error recovery possible
            qWarning() << "CouchDB: Network error" << r->error();
            return false;
        }

        bool ok;
        QByteArray rawdata = r->readAll();
        QVariantMap data = QJson::Parser().parse ( rawdata, &ok ).toMap();
        if (!ok) {
            // Response is not json: no error recovery possible
            qWarning() << "CouchDB: Json parser:" << rawdata;
            return false;
        }

        if ( !data.contains ( QLatin1String ( "db_name" ) ) ) {
            // Response is not expected without db_name: no error recovery possible
            qWarning() << "CouchDB: db_name not found";
            return false;
        }

        int doccount = data.value(QLatin1String("doc_count")).toInt();
        if (!doccount && !installPluginData(QLatin1String("_server"))) {
            // Initial data could not be installed
            qWarning() << "CouchDB: doc_count = 0";
            return false;
        }
        m_last_changes_seq_nr = data.value ( QLatin1String ( "update_seq" ),0 ).toInt();
    }

    { // get events
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/events" ) );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if (r->error() != QNetworkReply::NoError) {
            // Database events could not be read: no error recovery possible
            qWarning() << "CouchDB: _design/_server/_view/events";
            return false;
        }

        bool ok;
        QByteArray rawdata = r->readAll();
        QVariantMap data = QJson::Parser().parse ( rawdata, &ok ).toMap();
        if (!ok) {
            // Response is not json: no error recovery possible
            qWarning() << "Json parser:" << rawdata;
            return false;
        }
        if ( ok && data.contains ( QLatin1String ( "rows" ) ) ) {
            QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
            for ( int i=0;i<list.size();++i ) {
                data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
                emit couchDB_Event_add ( ServiceID::id(data), data );
            }
        }
    }

    // startChangeLister for settings
    {
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/events&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
        request.setRawHeader ( "Connection","keep-alive" );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange()) );
        connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorFatal(QNetworkReply::NetworkError)) );
    }
    // startChangeLister for settings
    {
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/settings&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
        request.setRawHeader ( "Connection","keep-alive" );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( readyRead() ), SLOT ( replyPluginSettingsChange() ) );
        connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorFatal(QNetworkReply::NetworkError)) );
    }
    emit couchDB_ready();
    return true;
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

void CouchDB::errorWithRecovery(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "CouchDB: Network error:" << e << r->url().toString();
}

void CouchDB::errorFatal(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "CouchDB: Fatal Network error:" << e << r->url().toString();
    emit couchDB_failed(r->url().toString());
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
                connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
            }
        }
    }
}

void CouchDB::requestActionsOfCollection(const QString& collecion_id)
{
    if (collecion_id.isEmpty())
        return;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/actions?key=\"%1\"#%1" ).arg ( collecion_id ) );

    QNetworkReply* r = get ( request );
    connect ( r, SIGNAL ( finished() ), SLOT ( replyActionOfCollection() ) );
    connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void CouchDB::requestPluginSettings(const QString& pluginid, bool tryToInstall)
{
    if (pluginid.isEmpty())
        return;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("pluginconfig_%1#%1" ).arg ( pluginid ) );

    QNetworkReply* r = get ( request );
    r->setProperty("tryToInstall", tryToInstall);
    r->setProperty("pluginid", pluginid);
    connect ( r, SIGNAL ( finished() ), SLOT ( replyPluginSettings() ) );
}

void CouchDB::replyPluginSettings()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    bool tryToInstall = r->property("tryToInstall").toBool();
    r->deleteLater();

    if ( r->error() != QNetworkReply::NoError ) {
        // settings not found, try to install initial plugin values to the couchdb
        if (tryToInstall) {
            const QString pluginid = r->property("pluginid").toString();
            installPluginData(pluginid);
            requestPluginSettings(pluginid, false);
        } else {
            emit couchDB_no_settings_found(r->url().fragment());
            return;
        }
    }

    while (r->canReadLine()) {
        QByteArray line = r->readLine();
        if ( line.isEmpty() ) return;
        bool ok;
        QVariantMap data = QJson::Parser().parse ( line, &ok ).toMap();
        if ( ok )
            emit couchDB_settings ( ServiceID::id(data), data );
        else {
            qWarning() << "Json parser:" << line;
            return;
        }
    }
}

void CouchDB::replyPluginSettingsChange()
{
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
                emit couchDB_no_settings_found(ServiceID::idChangeSeq ( data ));
            } else {
                requestPluginSettings(ServiceID::idChangeSeq ( data ));
            }
        }
    }
}

void CouchDB::errorNoSettings()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    r->deleteLater();
    emit couchDB_no_settings_found(r->url().fragment());
}


int CouchDB::installPluginData(const QString& pluginid) {
    int count = 0;
    qDebug() << "Install couchdb data for" << pluginid;
    const QDir dir = setup::pluginCouchDBDir(pluginid);
    {
        QEventLoop eventLoop;
        const QStringList files = dir.entryList(QStringList(QLatin1String("*.json")), QDir::Files|QDir::NoDotAndDotDot);
        for (int i=0;i<files.size();++i) {
            QFile file(dir.absoluteFilePath(files[i]));
            file.open(QIODevice::ReadOnly);
            if (file.size() > 1024*10) {
                qWarning() << "\tFile to big!" << files[i] << file.size();
                continue;
            }
            // Document ID: Consist of filename without extension + "{pluginid}".
            // "()" are replaced by "/".
            const QString doc_name = QFileInfo(files[i]).baseName().replace(QLatin1String("()"), QLatin1String("/")) + pluginid;
            QNetworkRequest request( setup::couchdbAbsoluteUrl( doc_name ) );
            request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
            request.setHeader(QNetworkRequest::ContentLengthHeader, file.size());
            QNetworkReply* rf = put(request, file.readAll());
            connect ( rf, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            file.close();
            if (rf->error() == QNetworkReply::NoError) {
                qDebug() << "\tInstalled" << doc_name << file.size();
                ++count;
            } else {
                qWarning() << "\tInstallation failed:" << doc_name << file.size() << rf->error();
            }
            delete rf;
        }
    }
    {
        QStringList filters;
        filters << QLatin1String("*.xml") << QLatin1String("*.html") << QLatin1String("*.htm");
        filters << QLatin1String("*.jpg") << QLatin1String("*.js") << QLatin1String("*.css");
        QStringList files = dir.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);
        QString revision;
        QEventLoop eventLoop;
        const QString doc_html = QLatin1String("html_")+pluginid;
        { // Get revision of htmlplugin_{pluginname} document where all additional files should be attached
            QNetworkRequest request( setup::couchdbAbsoluteUrl(doc_html) );
            QNetworkReply* rf = head(request);
            connect ( rf, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            if (rf->error() == QNetworkReply::NoError) {
                revision = QString::fromAscii(rf->rawHeader("Etag"));
            } else {
                qWarning() << "\tAttachment doc not found:" << doc_html << rf->error();
            }
            delete rf;
        }
        if (!revision.size())
            return count;

        for (int i=0;i<files.size();++i) {
            QFile file(dir.absoluteFilePath(files[i]));
            file.open(QIODevice::ReadOnly);
            if (file.size() > 1024*10) {
                qWarning() << "\tFile to big!" << files[i] << file.size();
                continue;
            }
            QNetworkRequest request( setup::couchdbAbsoluteUrl(doc_html+QLatin1String("/attachment?rev=")+revision) );
            request.setHeader(QNetworkRequest::ContentLengthHeader, file.size());
            const QByteArray suffix = QFileInfo(files[i]).suffix().toLatin1();
            if (suffix == "xml") {
                request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/xml"));
            } else if (suffix == "jpg") {
                request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));
            } else if (suffix == "html") {
                request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("text/html"));
            } else if (suffix == "htm") {
                request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("text/html"));
            } else if (suffix == "js") {
                request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-javascript"));
            } else if (suffix == "css") {
                request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("text/css"));
            } else {
                qWarning() << "\ttAttachment not recognized:" << files[i] << file.size();
                continue;
            }
            QNetworkReply* rf = put(request, file.readAll());
            connect ( rf, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            file.close();
            if (rf->error() == QNetworkReply::NoError) {
                qDebug() << "\tAttachment installed" << files[i] << file.size();
                ++count;
            } else {
                qWarning() << "\ttAttachment Installation failed:" << files[i] << file.size() << rf->error();
            }
            delete rf;
        }
    }
    return count;
}
