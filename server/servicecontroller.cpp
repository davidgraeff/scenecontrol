#include "servicecontroller.h"
#include <QCoreApplication>
#include <qprocess.h>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "boolstuff/BoolExprParser.h"
#include "paths.h"
#include "plugincontroller.h"

#define __FUNCTION__ __FUNCTION__

ServiceController::ServiceController () : m_plugincontroller ( 0 ) {
}

ServiceController::~ServiceController() {
    // server shutdown event
    {
        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "serverstate" );
        sc.setData ( "state", 0 );
        property_changed ( sc.getData() );
        const QList<QString> uids = m_state_events.value ( 0 ).toList();
        for ( int i=0;i<uids.size();++i ) {
            event_triggered ( uids[i] );
        }
    }

    qDeleteAll ( m_valid_services );
}

void ServiceController::setPluginController ( PluginController* pc ) {
    m_plugincontroller=pc;
}

void ServiceController::load ( bool service_dir_watcher ) {
    // files in {home}/roomcontrol/services
    QDir dir = serviceDir();
    const QStringList tfiles = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    for ( int i=0;i<tfiles.size();++i ) {
        directoryChanged ( dir.absoluteFilePath ( tfiles[i] ), true );
    }

    connect ( &m_dirwatcher, SIGNAL ( directoryChanged ( QString ) ), SLOT ( directoryChanged ( QString ) ) );
    if ( service_dir_watcher ) {
        m_dirwatcher.addPath ( serviceDir().absolutePath() );
    }

    emit dataReady();

    // stats
    qDebug() << "Available Services    :" << m_plugincontroller->knownServices();
    qDebug() << "Loaded    Services    :" << m_valid_services.size();
}

void ServiceController::directoryChanged ( QString file, bool loading ) {
    if ( !loading ) {
        if ( !QFile::exists ( file ) ) {
            qDebug() << "File removed:"<<file;
            QStringList list = file.split ( QLatin1String ( "." ) );
            if ( list.size() <3 ) return;
            removeService ( list[1] );
            return;
        }
        qDebug() << "File changed:"<<file;
    }

    QFile f ( file );
    f.open ( QIODevice::ReadOnly );
    if ( !f.isOpen() ) {
        qWarning() << "Couldn't open file" << file;
        return;
    }

    bool ok = true;
    QVariantMap result = QJson::Parser().parse ( &f, &ok ).toMap();
    f.close();
    if ( !ok ) {
        qWarning() << "Not a json file" << file;
        return;
    }

    changeService ( result, QString() );
}

void ServiceController::changeService ( const QVariantMap& unvalidatedData, const QString& sessionid ) {
    QVariantMap data = unvalidatedData;
    if ( !validateService ( data ) ) return;
    qDebug() << "valid data json" << data;

    if ( ServiceType::isExecutable ( data ) ) {
        if ( ServiceType::uniqueID ( data ).size() )
            executeActionByUID ( ServiceType::uniqueID ( data ), sessionid );
        else
            executeAction ( data, sessionid );
        return;
    }

    if ( ServiceType::isRemoveCmd ( data ) ) {
        removeService ( ServiceType::uniqueID ( data ) );
        return;
    }

    if ( ServiceType::uniqueID ( data ).isEmpty() ) {
        setUniqueID ( data );
    }

    ServiceStruct* service = m_valid_services.value ( ServiceType::uniqueID ( data ) );
    if ( !service ) service = new ServiceStruct();

    service->data = data;
    service->plugin = dynamic_cast<AbstractPlugin_services*> ( m_plugincontroller->getPlugin ( ServiceID::id ( data ) ) );

    m_valid_services.insert ( ServiceType::uniqueID ( data ),service );

    saveToDisk ( data );

    emit dataSync ( data );
}

bool ServiceController::validateService ( const QVariantMap& data ) {
    // check remove
    if ( ServiceType::isRemoveCmd ( data ) ) {
        return ( ServiceType::uniqueID ( data ).size() );
    } else  if ( ServiceType::isExecutable ( data ) && ServiceType::uniqueID ( data ).size() ) {
        return true;
    }

    QDomNode* node = 0;

    // check service/property id. collections do not have ids
    if ( ServiceID::id ( data ).size() ) {
        node = m_plugincontroller->getPluginDom ( ServiceID::gid ( data ) );
    } else if ( ServiceType::isCollection ( data ) ) {
        node = m_plugincontroller->getPluginDom ( QLatin1String ( "collection" ) );
    }

    if ( !node ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) <<". Data"<<data <<": No description for id found!";
        return false;
    }

    // check uid and add one if neccessary
    if ( !ServiceType::isExecutable ( data ) && !ServiceType::isCollection ( data ) && ServiceType::uniqueID ( data ).isEmpty() ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": No uid found!";
        return false;
    }

    // check type
    const QString type = node->nodeName();
    if ( ServiceType::isAction ( data ) && type != QLatin1String ( "action" ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be action but is not!";
        return false;
    }
    if ( ServiceType::isExecutable ( data ) && ( type != QLatin1String ( "action" ) && type != QLatin1String ( "execute" ) ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be action or execute but is not!";
        return false;
    }
    if ( ServiceType::isEvent ( data ) && type != QLatin1String ( "event" ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be event but is not!";
        return false;
    }
    if ( ServiceType::isCondition ( data ) && type != QLatin1String ( "condition" ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be condition but is not!";
        return false;
    }
    if ( ServiceType::isCollection ( data ) && type != QLatin1String ( "collection" ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be collection but is not!";
        return false;
    }
    if ( ServiceType::isNotification ( data ) && type != QLatin1String ( "notification" ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be notification but is not!";
        return false;
    }
    if ( ServiceType::isModelItem ( data ) && type != QLatin1String ( "model" ) ) {
        qWarning() << "Cannot verify"<<ServiceID::gid ( data ) << ": Should be model but is not!";
        return false;
    }

    // check id. collections do not have ids
    {
        QDomNamedNodeMap attr = node->attributes();
        const QString id = attr.namedItem ( QLatin1String ( "id" ) ).nodeValue();
        if ( !ServiceType::isCollection ( data ) && id != ServiceID::id ( data ) ) {
            qWarning() << "Cannot verify"<<id<<ServiceID::id ( data ) << ": No id found!";
            return false;
        }
    }

    // check if all xml child notes are also represented in data
    QDomNodeList childs = node->childNodes();
    for ( int i=0;i<childs.count();++i ) {
        QDomNode node = childs.item ( i );
        QString type = node.nodeName();
        if ( type == QLatin1String ( "#comment" ) ) continue;

        if ( type == QLatin1String ( "string" ) ) {
        } else if ( type == QLatin1String ( "int" ) ) {
        } else if ( type == QLatin1String ( "uint" ) ) {
        } else if ( type == QLatin1String ( "double" ) ) {
        } else if ( type == QLatin1String ( "enum" ) ) {
        } else if ( type == QLatin1String ( "time" ) ) {
        } else if ( type == QLatin1String ( "date" ) ) {
        } else if ( type == QLatin1String ( "url" ) ) {
        } else if ( type == QLatin1String ( "bool" ) ) {
        } else if ( type == QLatin1String ( "enum" ) ) {
		} else if ( type == QLatin1String ( "stringlist" ) ) {
        } else {
            qWarning() << "Cannot verify"<<ServiceID::gid ( data ) <<"with unique id"<<ServiceType::uniqueID ( data ) <<": Unknown type" << type;
            return false;
        }

        if ( !ServiceType::isCollection ( data ) ) {
            // check child id
            QDomNamedNodeMap attr = node.attributes();
            const QString id = attr.namedItem ( QLatin1String ( "id" ) ).nodeName();
            if ( !data.contains ( id ) ) {
                qWarning() << "Cannot verify"<<ServiceID::gid ( data ) <<"with unique id"<<ServiceType::uniqueID ( data ) <<": Entry"<<id;
                return false;
            }
        }
    }
    return true;
}

void ServiceController::setUniqueID ( QVariantMap& data ) {     // Generate unique ids amoung all existing ids
    QString nid;
    do {
        nid = QUuid::createUuid().toString().remove ( QLatin1Char ( '{' ) ).remove ( QLatin1Char ( '}' ) );
    } while ( m_valid_services.contains ( nid ) );
    ServiceType::setUniqueID ( data, nid );
}

void ServiceController::saveToDisk ( const QVariantMap& data ) {
    if ( ServiceType::isExecutable ( data ) ) {
        qWarning() << "Requested to save an immediately-to-execute action!";
        return;
    }
    if ( ServiceType::isRemoveCmd ( data ) ) {
        qWarning() << "Requested to save an about-to-be-removed action!";
        return;
    }

    const QString path = serviceFilename ( ServiceType::type ( data ), ServiceType::uniqueID ( data ) );

    QFile file ( path );
    if ( !file.open ( QIODevice::ReadWrite | QIODevice::Truncate ) ) {
        qWarning() << __FUNCTION__ << "Couldn't save to " << path;
        return;
    }

    const QByteArray json = QJson::Serializer().serialize ( data );
    if ( data.isEmpty() ) {
        qWarning() << __FUNCTION__ << "Saving failed to " << path;
    } else {
        file.write ( json );
    }
    file.close();
}

QString ServiceController::serviceFilename ( const QString& type, const QString& uid ) {
    return serviceDir().absoluteFilePath ( type + QLatin1String ( "." ) + uid );
}

void ServiceController::event_triggered ( const QString& event_id, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    emit eventTriggered ( event_id );
}

void ServiceController::execute_action ( const QVariantMap& data, const char* pluginid ) {
    // data from plugins. Check this before execution
    if ( !validateService ( data ) ) {
        qWarning() <<"Invalid data to be executed from"<<pluginid;
        return;
    }
    executeAction ( data, QString() );
}

void ServiceController::property_changed ( const QVariantMap& data, const QString& sessionid, const char* pluginid ) {
    Q_UNUSED ( pluginid );
    emit dataSync ( data, sessionid );
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

const QMap< QString, ServiceController::ServiceStruct* >& ServiceController::valid_services() const {
    return m_valid_services;
}

void ServiceController::removeServicesUsingPlugin ( const QString& pluginid ) {
    QMutableMapIterator<QString, ServiceStruct* > it ( m_valid_services );
    while ( it.hasNext() ) {
        it.next();
        AbstractPlugin* p = dynamic_cast<AbstractPlugin*> ( it.value()->plugin );
        if ( p && p->pluginid() == pluginid ) {
            it.remove();
        }
    }
}

void ServiceController::removeService ( const QString& uid ) {
    ServiceStruct* service = m_valid_services.take ( uid );
    if ( !service ) return;
    QVariantMap data = service->data;
    delete service;
    service = 0;

    const QString filename = serviceFilename ( ServiceType::type ( data ), uid );
    if ( !QFile::remove ( filename ) || QFileInfo ( filename ).exists() ) {
        qWarning() << "Couldn't remove file" << filename;
        return;
    }

    // collection: remove all childs
    if ( ServiceType::isCollection ( data ) ) {
        QSet<QString> uids;
        QVariantMap actions = MAP ( "actions" );
        for ( QVariantMap::const_iterator i=actions.begin();i!=actions.end();++i ) {
            uids.insert ( i->toString() );
        }
        QVariantList events = LIST ( "events" );
        for ( QVariantList::const_iterator i=events.begin();i!=events.end();++i ) {
            uids.insert ( i->toString() );
        }
        boolstuff::BoolExprParser parser;
        try {
            boolstuff::BoolExpr<std::string>* conditions = parser.parse ( DATA ( "conditions" ).toStdString() );
            std::set<std::string> vars;
            conditions->getTreeVariables ( vars, vars );
            for ( std::set<std::string>::const_iterator i=vars.begin();i!=vars.end();++i ) {
                uids.insert ( QString::fromStdString ( *i ) );
            }
        } catch ( boolstuff::BoolExprParser::Error ) {
        }


        for ( QSet<QString>::const_iterator uid=uids.begin();uid!=uids.end();++uid ) {
            removeService ( *uid );
        }
    }

    ServiceCreation sc = ServiceCreation::createRemoveByUidCmd ( uid );
    emit dataSync ( sc.getData() );
}

void ServiceController::executeAction ( const QVariantMap& data, const QString& sessionid ) {
    if ( !ServiceType::isExecutable ( data ) ) return;
    AbstractPlugin* plugin = m_plugincontroller->getPlugin ( ServiceID::pluginid ( data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data;
        return;
    }
    executeplugin->execute ( data, sessionid );
}

void ServiceController::executeActionByUID ( const QString& uid, const QString& sessionid ) {
    ServiceStruct* service = m_valid_services.value ( uid );
    if ( !service || !service->plugin ) return;
    service->plugin->execute ( service->data, sessionid );
}

ServiceController::ServiceStruct* ServiceController::service ( const QString& uid ) {
    return m_valid_services.value ( uid );
}

void ServiceController::sessionBegin ( const QString& sessionid ) {
    {
        // properties
        QMap<QString,PluginInfo*>::iterator index = m_plugincontroller->getPluginIterator();
        while ( AbstractPlugin_services* plugin = m_plugincontroller->nextServicePlugin ( index ) ) {
            QList<QVariantMap> properties = plugin->properties ( sessionid );
            for ( int i=0;i<properties.size();++i ) {
                const QVariantMap prop = properties[i];
                emit dataSync ( prop, sessionid );
            }
        }
    }

    // services
    for ( QMap<QString, ServiceStruct*>::const_iterator i=m_valid_services.begin();i!=m_valid_services.end();++i ) {
        const ServiceStruct* service = *i;
        emit dataSync ( service->data, sessionid );
    }

    {
        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID,  "transfer.complete" );
        emit dataSync ( sc.getData(), sessionid );
    }

    {
        // sessions
        QMap<QString,PluginInfo*>::iterator  index = m_plugincontroller->getPluginIterator();
        while ( AbstractPlugin_sessions* plugin = m_plugincontroller->nextSessionPlugin ( index ) ) {
            plugin->session_change ( sessionid, true );
        }
    }
}

void ServiceController::sessionFinished ( QString sessionid, bool timeout ) {
    {
        Q_UNUSED ( timeout );
        // sessions
        QMap<QString,PluginInfo*>::iterator  index = m_plugincontroller->getPluginIterator();
        while ( AbstractPlugin_sessions* plugin = m_plugincontroller->nextSessionPlugin ( index ) ) {
            plugin->session_change ( sessionid, false );
        }
    }
}

/////////////// AbstractPlugin, AbstractPlugin_services ///////////////

QList< QVariantMap > ServiceController::properties ( const QString& sessionid ) {
    Q_UNUSED ( sessionid );
    QList<QVariantMap> l;
    {
        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "serverstate" );
        sc.setData ( "state", 1 );
        sc.setData ( "version", QCoreApplication::applicationVersion() );
        l.append ( sc.getData() );
    }
    {
        QMap<QString,PluginInfo*>::iterator index = m_plugincontroller->getPluginIterator();
        QString plugins;
        while ( AbstractPlugin* plugin = m_plugincontroller->nextPlugin ( index ) ) {
            plugins.append ( plugin->pluginid() );
            plugins.append ( QLatin1String ( ";" ) );
        }
        plugins.chop ( 1 );
        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "plugininfo" );
        sc.setData ( "plugins", plugins );
        l.append ( sc.getData() );
    }
    return l;
}

bool ServiceController::condition ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( data );
    Q_UNUSED ( sessionid );
    return false;
}

void ServiceController::event_changed ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isId ( data,"serverevent" ) ) {
        // entfernen
        const QString uid = ServiceType::uniqueID ( data );
        QMutableMapIterator<int, QSet<QString> > it ( m_state_events );
        while ( it.hasNext() ) {
            it.next();
            it.value().remove ( uid );
            if ( it.value().isEmpty() )
                it.remove();
        }
        // hinzufügen
        m_state_events[INTDATA ( "state" ) ].insert ( uid );
    }
}

void ServiceController::execute ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isId ( data,"servercmd" ) ) {
        switch ( INTDATA ( "state" ) ) {
        case 0:
            QCoreApplication::exit ( 0 );
            break;
        default:
            break;
        }
    }
}

void ServiceController::initialize() {
    {
        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "serverstate" );
        sc.setData ( "state", 1 );
        sc.setData ( "version", QCoreApplication::applicationVersion() );
        property_changed ( sc.getData() );
    }

    const QList<QString> uids = m_state_events.value ( 1 ).toList();
    for ( int i=0;i<uids.size();++i ) {
        event_triggered ( uids[i] );
    }
}

void ServiceController::clear() {
    ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "serverstate" );
    sc.setData ( "state", 0 );
    sc.setData ( "version", QCoreApplication::applicationVersion() );
    property_changed ( sc.getData() );

    const QList<QString> uids = m_state_events.value ( 0 ).toList();
    for ( int i=0;i<uids.size();++i ) {
        event_triggered ( uids[i] );
    }
}
