#include "database.h"

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

static Database* databaseInstance = 0;

Database::Database () : m_last_changes_seq_nr ( 0 ), m_settingsChangeFailCounter(0), m_eventsChangeFailCounter(0) {
}

Database::~Database() {
}

Database* Database::instance()
{
    if (databaseInstance == 0)
        databaseInstance = new Database();

    return databaseInstance;
}

bool Database::connectToDatabase() {
    QEventLoop eventLoop;
    QNetworkReply *r;

    { // Connect to database (try to create it if not available)
        qDebug() << "Database:" << setup::couchdbAbsoluteUrl("" );
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
                qWarning() << "Database: Database not found and could not be created!";
                return false;
            }
            r = get ( request );
            connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            if (r->error() != QNetworkReply::NoError) {
                // Database created but could not be read: no error recovery possible
                qWarning() << "Database: Successfull created database but can not read it out!";
                return false;
            }
        } else if (r->error() != QNetworkReply::NoError) {
            // Network error: no error recovery possible
            qWarning() << "Database: Network error" << r->errorString();
            return false;
        }
    }

    { // try to parse database information
        QByteArray rawdata = r->readAll();
        QVariantMap data = JSON::parse ( rawdata ).toMap();
        if (data.isEmpty()) {
            // Response is not json: no error recovery possible
            qWarning() << "Database: Json parser:" << rawdata;
            return false;
        }

        if ( !data.contains ( QLatin1String ( "db_name" ) ) || !data.contains ( QLatin1String ( "doc_count" ) ) ) {
            // Response is not expected without db_name: no error recovery possible
            qWarning() << "Database: db_name or doc_count not found";
            return false;
        }

        int doccount = data.value(QLatin1String("doc_count")).toInt();
        if (!doccount && !installPluginData(QLatin1String("_server"))) {
            // Initial data could not be installed
            qWarning() << "Database: doc_count = 0";
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
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/events?key=\"%1\"#%1" ).arg ( plugin_id ) );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();
    if (r->error() != QNetworkReply::NoError) {
        // Database events could not be read: no error recovery possible
        qWarning() << "Database: Get events failed for" << plugin_id << r->error() << r->errorString();
        return;
    }

    QByteArray rawdata = r->readAll();
    QVariantMap data = JSON::parse ( rawdata ).toMap();
    if (data.isEmpty()) {
        // Response is not json: no error recovery possible
        qWarning() << "Database: Json parser:" << rawdata;
        return;
    }
    if (data.contains ( QLatin1String ( "rows" ) )) {
        QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        for ( int i=0;i<list.size();++i ) {
            data = list[i].toMap().value ( QLatin1String ( "value" ) ).toMap();
            data.remove(QLatin1String("_rev"));
            data.remove(QLatin1String("type_"));
            if ( ServiceData::collectionid ( data ).isEmpty() ) {
                qWarning() <<"Database: Received event without collection:"<<ServiceData::pluginid ( data ) << ServiceData::id ( data );
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
            qWarning() << "Database: replyEventsChange" << r->url() << r->errorString();
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
        QNetworkRequest request ( setup::couchdbAbsoluteUrl(docid));

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
            qWarning() <<"Database: Received event without collection:"<<ServiceData::pluginid ( document ) << ServiceData::id ( document );
            continue;
        }
        emit Event_add ( ServiceData::id(document), document );
    }
}

void Database::startChangeListenerEvents()
{
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/events&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyEventsChange()) );
    //connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void Database::startChangeListenerSettings()
{
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_changes?feed=continuous&since=%1&filter=_server/settings&heartbeat=5000" ).arg ( m_last_changes_seq_nr ) );
    request.setRawHeader ( "Connection","keep-alive" );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( readyRead() ), SLOT ( replyPluginSettingsChange() ) );
    //connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

bool Database::checkFailure(QNetworkReply* r)
{
    if ( r->error() != QNetworkReply::NoError ) {
        qWarning() << "Database: Response error:" << r->url();
        emit failed(r->url().toString());
        return true;
    }
    return false;
}

void Database::errorWithRecovery(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "Database: Network error:" << e << r->url().toString();
}

void Database::errorFatal(QNetworkReply::NetworkError e) {
    QNetworkReply *r = ( QNetworkReply* ) sender();
    qWarning() << "Database: Fatal Network error:" << e << r->url().toString();
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
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/services_actionsconditions?key=\"%1\"#%1" ).arg ( collecion_id ) );

    QNetworkReply* r = get ( request );
    connect ( r, SIGNAL ( finished() ), SLOT ( replyDataOfCollection()) );
    connect ( r, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorWithRecovery(QNetworkReply::NetworkError)) );
}

void Database::requestPluginSettings(const QString& pluginid, bool tryToInstall)
{
    if (pluginid.isEmpty())
        return;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_design/_server/_view/settings?key=\"%1\"" ).arg(pluginid));

    QNetworkReply* r = get ( request );
    QEventLoop eventLoop;
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();

    if ( r->error() != QNetworkReply::NoError) {
        qWarning()<<"Database: Get settings failed for" << pluginid << r->error() << r->errorString();
        return;
    }

    QVariantMap data = JSON::parse ( r->readAll() ).toMap();
    if ( !data.isEmpty() && data.contains ( QLatin1String ( "rows" ) ) ) {
        QVariantList list = data.value ( QLatin1String ( "rows" ) ).toList();
        if (list.isEmpty()) {
            if (!tryToInstall)
                return;
            // settings not found, try to install initial plugin values to the couchdb
            installPluginData(pluginid);
            delete r;
            r = get ( request );
            connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
            eventLoop.exec();
            if ( r->error() != QNetworkReply::NoError ) {
                qWarning()<<"Database: Get settings failed for" << pluginid << r->error() << r->errorString();
                delete r;
                return;
            }
            QVariantMap data = JSON::parse ( r->readAll() ).toMap();
            if ( data.isEmpty() || !data.contains ( QLatin1String ( "rows" ) ) ) {
                qWarning()<<"Database: Get settings failed for" << pluginid << data;
                return;
            }
            list = data.value ( QLatin1String ( "rows" ) ).toList();
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
        qWarning() << "Database: Get settings failed for" << pluginid << data;
    }

    delete r;
}

void Database::replyPluginSettingsChange()
{
    QNetworkReply *r = ( QNetworkReply* ) sender();
    if ( r->error() != QNetworkReply::NoError ) {
        ++m_settingsChangeFailCounter;
        if (m_settingsChangeFailCounter > 5) {
            qWarning() << "Database: replyPluginSettingsChange" << r->url();
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
//     emit no_settings_found(r->url().fragment());
}


int Database::installPluginData(const QString& pluginid) {
    int count = 0;
    QDir dir = setup::pluginCouchDBDir();
    if (!dir.cd(pluginid)) {
        qWarning()<<"Database: failed to change to " << dir.absolutePath() << pluginid;
        return 0;
    }

    QEventLoop eventLoop;
    const QStringList files = dir.entryList(QStringList(QLatin1String("*.json")), QDir::Files|QDir::NoDotAndDotDot);
    qDebug() << "Database: Install" << pluginid << "with" << files.size() << "json files";
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
        if (error) {
            qWarning() << "\tNot a json file although json file extension!";
            continue;
        }
        // Add plugin id before inserting into database
        jsonData[QLatin1String("plugin_")] = pluginid;
        const QByteArray dataToSend = JSON::stringify(jsonData).toUtf8();
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

    return count;
}

void Database::extractJSONFromCouchDB(const QString& path)
{
    qDebug() << "Database: Extract JSON Files to" << path;
    QEventLoop eventLoop;
    QNetworkRequest request ( setup::couchdbAbsoluteUrl("_all_docs") );
    QNetworkReply *r = get ( request );
    connect ( r, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
    eventLoop.exec();
    if (r->error() != QNetworkReply::NoError) {
        qWarning() << "Database: Failed to extract JSON Files";
        return;
    }

    QByteArray rawdata = r->readAll();
    delete r;
    QVariantMap jsonData = JSON::parse ( rawdata ).toMap();
    if (jsonData.isEmpty()) {
        // Response is not json: no error recovery possible
        qWarning() << "Database: Json parser:" << rawdata;
        return;
    }

    if (!jsonData.contains ( QLatin1String ( "rows" ) )) {
        // Response is not json: no error recovery possible
        qWarning() << "Database: Field rows not found";
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
            qWarning() << "Database: Extraction failed" << id;
            continue;
        }

        const QByteArray rawdata = r->readAll();
        QVariantMap jsonData = JSON::parse ( rawdata ).toMap();
        if (jsonData.isEmpty()) {
            qWarning() << "Database: Extraction failed (JSON!)" << id;
            continue;
        }
        QDir dir(path);
        if (jsonData.contains(QLatin1String("plugin_"))) {
            const QString pluginid = jsonData.value(QLatin1String("plugin_")).toString();
            if (!dir.cd(pluginid) && (!dir.mkdir(pluginid) || !dir.cd(pluginid))) {
                qWarning() << "Database: Failed to create subdir" << pluginid << dir;
                continue;
            }
        } else if (jsonData.value(QLatin1String("type_")).toString() == QLatin1String("collection")) {
            const QString pluginid = QLatin1String("collections");
            if (!dir.cd(pluginid) && (!dir.mkdir(pluginid) || !dir.cd(pluginid))) {
                qWarning() << "Database: Failed to create subdir" << pluginid << dir;
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

void Database::changePluginConfiguration(const QString& pluginid, const QString& key, const QVariantMap& value) {
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
                qWarning() << "Database: Json parser:" << rawdata;
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
        const QByteArray dataToSend = JSON::stringify(data).toUtf8();
        QNetworkRequest request( setup::couchdbAbsoluteUrl( docid ) );
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
        request.setHeader(QNetworkRequest::ContentLengthHeader, dataToSend.size());
        QNetworkReply* rf = put(request, dataToSend);
        connect ( rf, SIGNAL ( finished() ), &eventLoop, SLOT ( quit() ) );
        eventLoop.exec();
        rf->deleteLater();
    }
}
