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
#include <qeventloop.h>
#include <QTimer>
#include <shared/pluginservicehelper.h>
#include <shared/json.h>
#define __FUNCTION__ __FUNCTION__

static CouchDB* couchdbInstance = 0;

CouchDB::CouchDB () : m_last_changes_seq_nr ( 0 ), m_settingsChangeFailCounter(0), m_eventsChangeFailCounter(0) {
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
    QNetworkReply *r;

    { // Connect to database (try to create it if not available)
        qDebug() << "CouchDB:" << setup::couchdbAbsoluteUrl("" );
        QNetworkRequest request ( setup::couchdbAbsoluteUrl("" ) );
        r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if (r->error() == QNetworkReply::ContentNotFoundError) {
            // Create database by using HTTP PUT
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
        if (!doccount && !installPluginData(QLatin1String("_server"))) {
            // Initial data could not be installed
            qWarning() << "CouchDB: doc_count = 0";
            return false;
        }
        m_last_changes_seq_nr = data.value ( QLatin1String ( "update_seq" ),0 ).toInt();
    }

    emit couchDB_ready();
    return true;
}

void CouchDB::requestEvents()
{
    QEventLoop eventLoop;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/events" ) );
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
            emit couchDB_Event_add ( ServiceData::id(data), data );
        }
    }
}

void CouchDB::startChangeListenerEvents()
{
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/events&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange()) );
    //connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void CouchDB::startChangeListenerSettings()
{
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/settings&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyPluginSettingsChange() ) );
    //connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

bool CouchDB::checkFailure(QNetworkReply* r)
{
    if ( r->error() != QNetworkReply::NoError ) {
        qWarning() << "CouchDB: Response error:" << r->url();
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

void CouchDB::replyDataOfCollection()
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
        emit couchDB_dataOfCollection( actionsList, conditionList, r->url().fragment() );
    }
}

void CouchDB::replyEventsChange() {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    if ( r->error() != QNetworkReply::NoError ) {
        ++m_eventsChangeFailCounter;
        if (m_eventsChangeFailCounter > 5) {
            qWarning() << "CouchDB: replyEventsChange" << r->url();
        } else {
            QTimer::singleShot(1000, this, SLOT(replyEventsChange()));
        }
        delete r;
        return;
    }

    m_eventsChangeFailCounter = 0;

    while ( r->canReadLine() ) {
        const QByteArray line = r->readLine();
        if ( line.size() <= 1 ) continue;
        QVariantMap data = JSON::parse ( line ).toMap();
        if ( !data.isEmpty() && data.contains ( QLatin1String ( "seq" ) ) ) {
            const int seq = data.value ( QLatin1String ( "seq" ) ).toInt();
            if ( seq > m_last_changes_seq_nr )
                m_last_changes_seq_nr = seq;
            if ( data.contains ( QLatin1String ( "deleted" ) ) ) {
                emit couchDB_Event_remove(ServiceData::idChangeSeq ( data ));
            } else {
                const QNetworkRequest request ( setup::couchdbAbsoluteUrl("%1" ).arg ( data[QLatin1String ( "id" ) ].toString() ) );
                QNetworkReply* r = get ( request );
                connect ( r, SIGNAL ( finished() ), SLOT ( replyEvent() ) );
                connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
            }
        }
    }
}

void CouchDB::requestDataOfCollection(const QString& collecion_id)
{
    if (collecion_id.isEmpty())
        return;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/actionsconditions?key=\"%1\"#%1" ).arg ( collecion_id ) );

    QNetworkReply* r = get ( request );
    connect ( r, SIGNAL ( finished() ), SLOT ( replyDataOfCollection()) );
    connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void CouchDB::requestPluginSettings(const QString& pluginid, bool tryToInstall)
{
    if (pluginid.isEmpty())
        return;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/settings?key=\"%1\"" ).arg(pluginid));

    QNetworkReply* r = get ( request );
    QEventLoop eventLoop;
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();

    if ( r->error() != QNetworkReply::NoError ) {
        if (!tryToInstall)
            return;
        // settings not found, try to install initial plugin values to the couchdb
        installPluginData(pluginid);
        delete r;
        r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if ( r->error() != QNetworkReply::NoError ) {
            qWarning()<<"CouchDB: Get settings failed for" << pluginid;
            delete r;
            return;
        }
    }

    QVariantMap data = JSON::parse ( r->readAll() ).toMap();
    if ( !data.isEmpty() && data.contains ( QLatin1String ( "rows" ) ) ) {
        const QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        foreach(const QVariant& v, list) {
            data = v.toMap();
            data.remove(QLatin1String("_id"));
            data.remove(QLatin1String("_rev"));
            data.remove(QLatin1String("type_"));
            data.remove(QLatin1String("plugin_"));
            const QString key = data.take(QLatin1String("key_")).toString();
            emit couchDB_settings ( pluginid, key, data );
        }
    } else {
        qWarning() << "CouchDB: Get settings failed for" << pluginid << data;
    }

    delete r;
}

void CouchDB::replyPluginSettingsChange()
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
        QNetworkRequest request ( setup::couchdbAbsoluteUrl(docid));

        QNetworkReply* r = get ( request );
        QEventLoop eventLoop;
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();

        QVariantMap document = JSON::parse(r->readAll() ).toMap();

        if (!ServiceData::checkType(document, ServiceData::TypeConfiguration))
            continue;
        document.remove(QLatin1String("_id"));
        document.remove(QLatin1String("_rev"));
        document.remove(QLatin1String("type_"));
        document.remove(QLatin1String("plugin_"));
        const QString key = data.take(QLatin1String("key_")).toString();
        emit couchDB_settings ( ServiceData::pluginid(document), key, document );
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
    QDir dir = setup::pluginCouchDBDir();
    if (!dir.cd(pluginid)) {
        qWarning()<<"CouchDB: failed to change to " << dir.absolutePath() << pluginid;
        return 0;
    }
    {
        QEventLoop eventLoop;
        const QStringList files = dir.entryList(QStringList(QLatin1String("*.json")), QDir::Files|QDir::NoDotAndDotDot);
        qDebug() << "CouchDB: Install" << pluginid << "with" << files.size() << "json files";
        for (int i=0;i<files.size();++i) {
            QFile file(dir.absoluteFilePath(files[i]));
            file.open(QIODevice::ReadOnly);
            if (file.size() > 1024*10) {
                qWarning() << "\tFile to big!" << files[i] << file.size();
                continue;
            }
            bool error=false;
            QTextStream stream( &file );
            QVariantMap jsonData = JSON::parseValue(stream, error).toMap();
            if (!error) {
                qWarning() << "\tNot a json file although json file extension!";
                continue;
            }
            // Add plugin id before inserting into database
            jsonData[QLatin1String("plugin_")] = pluginid;
            const QByteArray dataToSend = JSON::stringify(jsonData);

            // Document ID: Consist of filename without extension + "{pluginid}".
            // "()" are replaced by "/".
            const QString docid = QFileInfo(files[i]).baseName().replace(QLatin1String("()"), QLatin1String("/")) + pluginid;
            QNetworkRequest request( setup::couchdbAbsoluteUrl( docid ) );
            request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
            request.setHeader(QNetworkRequest::ContentLengthHeader, dataToSend.size());
            QNetworkReply* rf = put(request, dataToSend);
            connect ( rf, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            file.close();
            if (rf->error() == QNetworkReply::NoError) {
                qDebug() << "\tInstalled" << docid << ", entries:" << jsonData.size();
                ++count;
            } else {
                qWarning() << "\tInstallation failed (name, file size, entries, error):" << docid << file.size() << jsonData.size() << rf->error();
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

void CouchDB::extractJSONFromCouchDB(const QString& path)
{
    qDebug() << "CouchDB: Extract JSON Files to" << path;
    QEventLoop eventLoop;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_all_docs") );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();
    if (r->error() != QNetworkReply::NoError) {
        qWarning() << "CouchDB: Failed to extract JSON Files";
        return;
    }

    QByteArray rawdata = r->readAll();
    delete r;
    QVariantMap jsonData = JSON::parse ( rawdata ).toMap();
    if (jsonData.isEmpty()) {
        // Response is not json: no error recovery possible
        qWarning() << "CouchDB: Json parser:" << rawdata;
        return;
    }

    if (!jsonData.contains ( QLatin1String ( "rows" ) )) {
        // Response is not json: no error recovery possible
        qWarning() << "CouchDB: Field rows not found";
        return;
    }

    QVariantList list = jsonData.value ( QLatin1String ( "rows" ) ).toList();
    for ( int i=0;i<list.size();++i ) {
        const QString id = list[i].toMap().value ( QLatin1String ( "id" ) ).toString();
        QNetworkRequest request ( setup::couchdbAbsoluteUrl(id) );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if (r->error() != QNetworkReply::NoError) {
            qWarning() << "CouchDB: Extraction failed" << id;
            continue;
        }

        const QByteArray rawdata = r->readAll();
        QVariantMap jsonData = JSON::parse ( rawdata ).toMap();
        if (jsonData.isEmpty()) {
            qWarning() << "CouchDB: Extraction failed (JSON!)" << id;
            continue;
        }
        QDir dir(path);
        if (jsonData.contains(QLatin1String("plugin_"))) {
            const QString pluginid = jsonData.value(QLatin1String("plugin_")).toString();
            if (!dir.cd(pluginid) && (!dir.mkdir(pluginid) || !dir.cd(pluginid))) {
                qWarning() << "CouchDB: Failed to create subdir" << pluginid << dir;
                continue;
            }
        } else if (jsonData.value(QLatin1String("type_")).toString() == QLatin1String("collection")) {
            const QString pluginid = QLatin1String("collections");
            if (!dir.cd(pluginid) && (!dir.mkdir(pluginid) || !dir.cd(pluginid))) {
                qWarning() << "CouchDB: Failed to create subdir" << pluginid << dir;
                continue;
            }
        }

        QFile f(dir.absoluteFilePath(jsonData.value(QLatin1String("type_")).toString()+id+QLatin1String(".json")));
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(rawdata);
        f.close();
        delete r;
    }
}

void CouchDB::changePluginConfiguration(const QString& pluginid, const QString& key, const QVariantMap& value) {
    QVariantMap data;
    QEventLoop eventLoop;
    const QString docid = QLatin1String("configplugin_") + pluginid + QLatin1String("_") + key;
    // 1) try to get old settings document
    {
        QNetworkRequest request ( setup::couchdbAbsoluteUrl(docid) );
        QNetworkReply *r = get ( request );
        connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        if (r->error() == QNetworkReply::NoError) {
            QByteArray rawdata = r->readAll();
            delete r;
            data = JSON::parse ( rawdata ).toMap();
            if (!data.isEmpty()) {
                // Response is not json: no error recovery possible
                qWarning() << "CouchDB: Json parser:" << rawdata;
                return;
            }
        }
    }

    // 2) save new data
    {
        const QString rev = data.value(QLatin1String("_rev")).toString();
        data = value;
        if (rev.size())
            data[QLatin1String("_rev")] = rev;
        data[QLatin1String("_id")] = docid;
    }

    // 3) send to couchdb
    {
        const QByteArray dataToSend = JSON::stringify(data);
        QNetworkRequest request( setup::couchdbAbsoluteUrl( docid ) );
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
        request.setHeader(QNetworkRequest::ContentLengthHeader, dataToSend.size());
        QNetworkReply* rf = put(request, dataToSend);
        connect ( rf, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        rf->deleteLater();
    }
}
