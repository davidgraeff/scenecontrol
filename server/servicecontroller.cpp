#include "servicecontroller.h"
#include <QCoreApplication>
#include <qprocess.h>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>

#include "shared/categorize/profile.h"
#include "shared/categorize/category.h"
#include "shared/qjson/parser.h"
#include "shared/qjson/qobjecthelper.h"
#include "shared/qjson/serializer.h"
#include "shared/abstractplugin.h"
#include "shared/abstractstatetracker.h"
#include "shared/server/executeservice.h"
#include "shared/server/executeplugin.h"
#include "executeprofile.h"
#include <QApplication>
#include <plugins/coreplugin/services_server/systemACServer.h>
#include <plugins/coreplugin/services/systemAC.h>
#include <plugins/coreplugin/services_server/systemEVServer.h>
#include <plugins/coreplugin/services_server/profileACServer.h>
#include <plugins/coreplugin/services/profileAC.h>
#define __FUNCTION__ __FUNCTION__

ServiceController::ServiceController ()
{
    int offered_services = 0;
    ExecutePlugin *plugin;

    const QDir pluginsDir = QDir(qApp->property("pluginspath").value<QString>());
    const QStringList plugins = pluginsDir.entryList ( QDir::Files );
    foreach ( QString fileName, plugins )
    {
        QPluginLoader* loader = new QPluginLoader ( pluginsDir.absoluteFilePath ( fileName ), this );
        loader->setLoadHints(QLibrary::ResolveAllSymbolsHint);
        if (!loader->load()) {
            qWarning() << "Start: Failed loading" << pluginsDir.absoluteFilePath ( fileName ) << loader->errorString();
            delete loader;
            continue;
        }
        plugin = dynamic_cast<ExecutePlugin*> ( loader->instance() );
        if (!plugin) {
            qWarning() << "Start: Failed to get instance" << pluginsDir.absoluteFilePath ( fileName );
            delete loader;
            continue;
        }
        qDebug() << "Start: Load Plugin"<<plugin->base()->name() <<plugin->base()->version();

        connect(plugin,SIGNAL(stateChanged(AbstractStateTracker*)),SIGNAL(statetrackerSync(AbstractStateTracker*)));
        connect(this,SIGNAL(serviceSync(AbstractServiceProvider*)),plugin,SIGNAL(_serviceChanged(AbstractServiceProvider*)));
        connect(plugin,SIGNAL(executeService(AbstractServiceProvider*)),SLOT(pluginexecuteService(AbstractServiceProvider*)));
        connect(plugin,SIGNAL(pluginLoadingComplete(ExecutePlugin*)),SLOT(pluginLoadingComplete(ExecutePlugin*)));
        m_plugins.append ( plugin );
        QStringList provides = plugin->base()->registerServices();
        foreach ( QString provide, provides )
        {
            m_plugin_provider.insert ( provide, plugin );
        }
        offered_services += provides.size();
    }

    qDebug() << "Start: Load service provider";
    m_savedir = QDir(qApp->property("settingspath").value<QString>());
    QStringList files = m_savedir.entryList ( QDir::Files );
    QJson::Parser parser;
    foreach ( QString file, files )
    {
        bool ok = true;
        QFile f ( m_savedir.absoluteFilePath ( file ) );
        f.open ( QIODevice::ReadOnly );
        if ( !f.isOpen() )
        {
            qWarning() << "Couldn't open file" << file;
            continue;
        }

        QVariantMap result = parser.parse ( &f, &ok ).toMap();
        f.close();
        if ( !ok )
        {
            qWarning() << "Not a json file" << file;
            continue;
        }

        if ( !generate ( result, true ) )
        {
            qWarning() << "Json file content not recognised" << file;
            continue;
        }
    }

    // link execute_services to execute_collections
    foreach (ExecuteWithBase* s, m_servicesList) {
        // do not look at collections, only at services
        ExecuteService* exservice = dynamic_cast<ExecuteService*>(s);
        if (exservice) addToExecuteProfiles(exservice);
    }

    emit systemStarted();

    // stats
    qDebug() << "Available StateTracker:" << stateTracker().size();
    qDebug() << "Available Services    :" << offered_services;
    qDebug() << "Loaded    Services    :" << m_servicesList.size();
}

ServiceController::~ServiceController()
{
    foreach ( ExecutePlugin* plugin, m_plugins )
    {
        plugin->clear();
    }
    qDeleteAll(m_servicesList);
    qDeleteAll(m_plugins);
}

QList<AbstractStateTracker*> ServiceController::stateTracker()
{
    QList<AbstractStateTracker*> l;
    foreach ( ExecutePlugin* plugin, m_plugins )
    {
        l.append ( plugin->stateTracker() );
    }
    return l;
}

bool ServiceController::generate ( const QVariantMap& json, bool loading )
{
    const QString id = json.value ( QLatin1String ( "id" ) ).toString();
    if (loading && id.isEmpty()) {
        const QString filename = serviceFilename(json.value ( QLatin1String ( "type" ) ).toByteArray(), id);
        QFile::remove(filename);
        qWarning() << "Invalid service file detected and removed" << filename;
    }
    ExecuteWithBase* service = m_services.value ( id );
    bool remove = json.contains ( QLatin1String ( "remove" ) );
    bool iExecute = json.contains ( QLatin1String ( "iexecute" ) );

    if ( service )
    {
        QJson::QObjectHelper::qvariant2qobject ( json, service->base() );
        if ( remove )
        {
            removeFromDisk ( service );
            return true;
        }
        else if (!iExecute )
        {
            updateService(service);
            return true;
        }
        return false;
    }
    else if ( remove ) return false;

    // Object with id not known. Create new object
    const QString type = json.value ( QLatin1String ( "type" ), QString() ).toString();
    if ( type.isEmpty())
    {
        qWarning() << __FUNCTION__ << "detected json object without type" << json;
        return false;
    }

    if (type.toAscii() == Category::staticMetaObject.className()) {
        service = new ExecuteWithBase(new Category(), this);
    } else if (type.toAscii() == Collection::staticMetaObject.className()) {
        service = new ExecuteCollection(new Collection(), this);
    } else {
        ExecutePlugin* eplugin = m_plugin_provider.value ( type );
        if ( !eplugin )
        {
            qWarning() << __FUNCTION__ << "no plugin for json object" << json;
            return false;
        }

        service=eplugin->createExecuteService ( type ) ;
    }

    if ( !service )
    {
        qWarning() << __FUNCTION__ << "no service from plugin for json object" << json;
        return false;
    }

    QJson::QObjectHelper::qvariant2qobject ( json, service->base() );

    // check if it is an immediately to execute actor
    if ( iExecute )
    {
        ExecuteService* exservice = dynamic_cast<ExecuteService*>(service);
        Q_ASSERT(exservice);
        executeservice(exservice);
        delete exservice;
        return true;
    }

    // send systemStarted event to corePlugin service
    if (service->metaObject()->className() == EventSystemServer::staticMetaObject.className()) {
        connect(this, SIGNAL(systemStarted()), (EventSystemServer*)service, SLOT(systemStarted()));
    }

    ExecuteCollection* collection = dynamic_cast<ExecuteCollection*>(service);
    if (collection) {
        // connect executecollection signals
        connect(collection,SIGNAL(executeservice(ExecuteService*)),SLOT(executeservice(ExecuteService*)));
		// to update names e.g. coreplugin:execute collection needs the collection names
        foreach(ExecutePlugin* plugin, m_plugins) {
			plugin->serverserviceChanged(collection->base());
        }
    }

    updateService(service, true, loading);
    m_services.insert ( service->base()->id(), service );
    m_servicesList.append ( service );
    return true;
}

void ServiceController::updateService(ExecuteWithBase* service, bool newid, bool loading)
{
    // Generate uinque ids amoung all existing ids
    if (newid && !loading) {
        QString nid;
        do {
            nid = QUuid::createUuid().toString().remove ( QLatin1Char ( '{' ) ).remove ( QLatin1Char ( '}' ) );
        } while (m_services.contains ( nid ));
        service->base()->setId ( nid );
    }

    ExecuteService* exservice = dynamic_cast<ExecuteService*>(service);
    if (exservice && !loading) {
        removeFromExecuteProfiles ( exservice );
        addToExecuteProfiles ( exservice );
    }
    if (exservice) {
        exservice->dataUpdate();
        exservice->nameUpdate();
    }
    if (!loading) saveToDisk ( service );
}

void ServiceController::removeFromDisk ( ExecuteWithBase* eservice )
{
    AbstractServiceProvider* service = eservice->base();
    // set dynamic property remove to true
    service->setProperty ( "remove",true );
    // propagate to all clients, so that those remove this provider, too.
    emit serviceSync ( service );

    if ( !QFile::remove ( serviceFilename ( service->type(), service->id() ) ) ||
            QFileInfo(serviceFilename ( service->type(), service->id() )).exists() )
        qWarning() << "Couldn't remove file" << serviceFilename ( service->type(), service->id() );

    // collection: remove all childs
    if (service->type() == Collection::staticMetaObject.className()) {
        // find executecollection
        ExecuteCollection* p = dynamic_cast<ExecuteCollection*>(eservice);
        Q_ASSERT(p);
        QSet<ExecuteService*> childs_linked = p->m_childs_linked;
        p->m_childs_linked.clear();
        foreach (ExecuteService* s, childs_linked) {
            removeFromDisk(s);
        }
    } else if (service->type() == Category::staticMetaObject.className()) {
        // go through every service and if it belongs to the to-be-deleted categorie then change its parentid
        foreach(ExecuteWithBase* s, m_servicesList) {
            if (s->base()->parentid() == service->id()) {
                s->base()->setParentid(QString());
                saveToDisk(s);
                emit serviceSync ( s->base() );
            }
        }
    } else {
        ExecuteService* exservice = dynamic_cast<ExecuteService*>(eservice);
        // Not true for categories that are only ExecuteWithBase
        if (exservice) removeFromExecuteProfiles ( exservice );
    }
    m_services.remove(service->id());
    m_servicesList.removeAll(eservice);
    eservice->deleteLater();
}

void ServiceController::saveToDisk ( ExecuteWithBase* eservice )
{
    AbstractServiceProvider* service = eservice->base();
    Q_ASSERT ( service );
    if ( service->dynamicPropertyNames().contains ( "iexecute" ) )
    {
        qWarning() << "Requested to save an immediately-to-execute action!";
        return;
    }
    if ( service->dynamicPropertyNames().contains ( "remove" ) )
    {
        qWarning() << "Requested to save an about-to-be-removed action!";
        return;
    }

    const QString path = serviceFilename ( service->type(), service->id() );

    QFile file ( path );
    if ( !file.open ( QIODevice::ReadWrite | QIODevice::Truncate ) )
    {
        qWarning() << __FUNCTION__ << "Couldn't save to " << path;
        return;
    }
    const QVariant variant = QJson::QObjectHelper::qobject2qvariant ( service );

    const QByteArray json = QJson::Serializer().serialize ( variant );
    if ( json.isNull() )
    {
        qWarning() << __FUNCTION__ << "Saving failed to " << path;
    }
    else
    {
        file.write ( json );
    }
    file.close();
    //qDebug() << __FUNCTION__ << json;
    emit serviceSync ( service );
}

QString ServiceController::serviceFilename ( const QByteArray& type, const QString& id )
{
    return m_savedir.absoluteFilePath ( QString::fromAscii ( type +"-" ) + id );
}


void ServiceController::refresh()
{
    foreach ( ExecutePlugin* plugin, m_plugins )
    {
        plugin->refresh();
    }
}

void ServiceController::addToExecuteProfiles ( ExecuteService* service )
{
    if (service->base()->parentid().isEmpty()) return;
    ExecuteCollection* p = dynamic_cast<ExecuteCollection*>(m_services.value(service->base()->parentid()));
    if (!p) return;
    p->registerChild ( service );
}

void ServiceController::removeFromExecuteProfiles ( ExecuteService* service )
{
    if (service->base()->parentid().isEmpty()) return;
    ExecuteCollection* p = dynamic_cast<ExecuteCollection*>(m_services.value(service->base()->parentid()));
    if (!p) return;
    p->removeChild(service);
}

void ServiceController::runProfile ( const QString& id, bool ignoreConditions )   // only run if enabled
{
    ExecuteCollection* p = dynamic_cast<ExecuteCollection*>(m_services.value(id));
    if (!p) return;
    p->run(ignoreConditions);
}

void ServiceController::stopProfile(const QString& id) {
    ExecuteCollection* p = dynamic_cast<ExecuteCollection*>(m_services.value(id));
    if (!p) return;
    p->stop();
}

void ServiceController::pluginexecuteService(AbstractServiceProvider* service) {
    Q_ASSERT(service);
    service->setProperty("iexecute",1);
    generate(QJson::QObjectHelper::qobject2qvariant(service));
}

void ServiceController::executeservice(ExecuteService* service) {
    if (service->base()->type() == ActorSystem::staticMetaObject.className()) {

        switch (((ActorSystem*)service->base())->action()) {
        case ActorSystem::RestartSystem:
            QCoreApplication::exit(EXIT_WITH_RESTART);
            break;
        case ActorSystem::QuitSystem:
            QCoreApplication::exit(0);
            break;
        case ActorSystem::ResyncSystem:
            refresh();
            break;
        default:
            break;
        };
    }
    if (service->base()->type() == ActorCollection::staticMetaObject.className()) {
        switch (((ActorCollection*)service->base())->action()) {
        case ActorCollection::StartProfile:
            runProfile(((ActorCollection*)service->base())->profileid(),true);
            break;
        case ActorCollection::CancelProfile:
            stopProfile(((ActorCollection*)service->base())->profileid());
            break;
        default:
            break;
        };
    }
    else
        service->execute();
}

void ServiceController::pluginLoadingComplete(ExecutePlugin* plugin) {
    for (int i=0;i<m_servicesList.size();++i) {
        if (m_plugin_provider.value(QString::fromAscii(m_servicesList[i]->base()->type())) != plugin) continue;
        ExecuteService* exservice = dynamic_cast<ExecuteService*>(m_servicesList[i]);
        if (exservice) {
            exservice->nameUpdate();
            emit serviceSync ( exservice->base() );
        }
    }
}
