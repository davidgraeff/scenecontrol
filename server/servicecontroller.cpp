#include "servicecontroller.h"
#include <QCoreApplication>
#include <qprocess.h>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>

#include "shared/services/profile.h"
#include "shared/qjson/parser.h"
#include "shared/qjson/qobjecthelper.h"
#include "shared/qjson/serializer.h"
#include "shared/services/category.h"
#include "shared/abstractplugin.h"
#include "shared/abstractstatetracker.h"
#include "shared/server/executeservice.h"
#include "shared/server/executeplugin.h"
#include "executeprofile.h"
#include <QApplication>
#define __FUNCTION__ __FUNCTION__

ServiceController::ServiceController ()
{
    int offered_services = 0;
    ExecutePlugin *plugin;
    qDebug() << "Start: Load Plugins";

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
        connect(plugin,SIGNAL(pluginobjectChanged(ExecuteWithBase*)),SLOT(pluginobjectChanged(ExecuteWithBase*)));
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
        if ( !ok )
        {
            qWarning() << "Not a json file" << file;
            continue;
        }

        if ( !generate ( result ) )
        {
            qWarning() << "Json file content not recognised" << file;
            continue;
        }
    }

    // link execute_services to execute_collections
    foreach (ExecuteWithBase* s, m_servicesList) {
        // do not look at collections, only at services
        ExecuteService* exservice = qobject_cast<ExecuteService*>(s);
        if (exservice) addToExecuteProfiles(exservice);
    }

    emit systemStarted();

    // stats
    qDebug() << "StateTracker   :" << stateTracker().size();
    qDebug() << "Services       :" << offered_services;
    qDebug() << "Loaded Services:" << m_servicesList.size();
}

ServiceController::~ServiceController()
{
    qDebug() << "Shutdown: ServiceController (Services)";
    qDeleteAll ( m_servicesList );
    qDebug() << "Shutdown: ServiceController (Plugins)";
    qDeleteAll ( m_plugins );
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

bool ServiceController::generate ( const QVariantMap& json )
{
    const QString id = json.value ( QLatin1String ( "id" ) ).toString();
    ExecuteWithBase* service = m_services.value ( id );
    bool remove = json.contains ( QLatin1String ( "remove" ) );

    if ( service )
    {
        if ( remove )
        {
            removeFromDisk ( service );
        }
        else
        {
            updateService(service, json, false);
        }
    }
    else if ( remove ) return false;

    // Object with id not known. Create new object
    const QString type = json.value ( QLatin1String ( "type" ), QString() ).toString();
    if ( type.isEmpty())
    {
        qWarning() << __FUNCTION__ << "detected json object without type" << json;
        return false;
    }

    ExecutePlugin* eplugin = m_plugin_provider.value ( type );
    if ( !eplugin )
    {
        qWarning() << __FUNCTION__ << "no plugin for json object" << json;
        return false;
    }

    service=eplugin->createExecuteService ( type ) ;
    if ( !service )
    {
        qWarning() << __FUNCTION__ << "no service from plugin for json object" << json;
        return false;
    }

    // check if it is an immediately to execute actor
    if ( json.contains ( QLatin1String ( "iexecute" ) ) )
    {
        ExecuteService* exservice = qobject_cast<ExecuteService*>(service);
        Q_ASSERT(exservice);
        exservice->execute();
        delete exservice;
    }
    else
    {
        updateService(service, json, true);
        m_services.insert ( service->base()->id(), service );
        m_servicesList.append ( service );
    }
    return true;
}

void ServiceController::updateService(ExecuteWithBase* service, const QVariantMap& json, bool newid)
{
    ExecuteService* exservice = qobject_cast<ExecuteService*>(service);
    if (exservice) removeFromExecuteProfiles ( exservice );

    // load json data into object
    QJson::QObjectHelper::qvariant2qobject ( json, service );

    // Generate uinque ids amoung all existing ids
    if (newid) {
        QString nid = json.value ( QLatin1String ( "id" ), QString() ).toString();
        while (m_services.contains ( nid ))
        {
            nid = QUuid::createUuid().toString().remove ( QLatin1Char ( '{' ) ).remove ( QLatin1Char ( '}' ) );
        };
        service->base()->setId ( nid );
    }

    if (exservice) {
        exservice->dataUpdate();
        addToExecuteProfiles ( exservice );
    }
    saveToDisk ( service );
}

void ServiceController::pluginobjectChanged(ExecuteWithBase* service) {
    if ( service->property("remove").isValid() )
    {
        removeFromDisk ( service );
    }
    else if (!m_services.contains ( service->base()->id() ))
    {
        m_services.insert ( service->base()->id(), service );
        m_servicesList.append ( service );
    }
}

void ServiceController::removeFromDisk ( ExecuteWithBase* service )
{
    // set dynamic property remove to true
    service->setProperty ( "remove",true );
    // propagate to all clients, so that those remove this provider, too.
    emit serviceSync ( service->base() );

    if ( !QFile::remove ( serviceFilename ( service->base() ) ) )
        qWarning() << "Couldn't remove file" << serviceFilename ( service->base() );

    // collection: remove all childs
    if (service->base()->type() == Collection::staticMetaObject.className()) {
        // find executecollection
        ExecuteCollection* p = qobject_cast<ExecuteCollection*>(service);
        Q_ASSERT(p);
        QSet<ExecuteService*> childs_linked = p->m_childs_linked;
        p->m_childs_linked.clear();
        foreach (ExecuteService* s, childs_linked) {
            removeFromDisk(s);
        }
    } else {
        ExecuteService* exservice = qobject_cast<ExecuteService*>(service);
        Q_ASSERT(exservice);
        removeFromExecuteProfiles ( exservice );
    }
    m_services.remove(service->base()->id());
    m_servicesList.removeAll(service);
    service->deleteLater();
}

void ServiceController::saveToDisk ( ExecuteWithBase* service )
{
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

    const QString path = serviceFilename ( service->base() );

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
    emit serviceSync ( service->base() );
}

QString ServiceController::serviceFilename ( AbstractServiceProvider* service )
{
    Q_ASSERT(service);
    return m_savedir.absoluteFilePath ( QString::fromAscii ( service->type() +"-" ) + service->id() );
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
    ExecuteCollection* p = qobject_cast<ExecuteCollection*>(m_services.value(service->base()->parentid()));
    if (!p) return;
    p->registerChild ( service );
}

void ServiceController::removeFromExecuteProfiles ( ExecuteService* service )
{
    if (service->base()->parentid().isEmpty()) return;
    ExecuteCollection* p = qobject_cast<ExecuteCollection*>(m_services.value(service->base()->parentid()));
    if (!p) return;
    p->removeChild(service);
}

void ServiceController::runProfile ( const QString& id )   // only run if enabled
{
    ExecuteCollection* p = qobject_cast<ExecuteCollection*>(m_services.value(id));
    if (!p) return;
    p->run();
}

void ServiceController::stopProfile(const QString& id) {
    ExecuteCollection* p = qobject_cast<ExecuteCollection*>(m_services.value(id));
    if (!p) return;
    p->stop();
}
