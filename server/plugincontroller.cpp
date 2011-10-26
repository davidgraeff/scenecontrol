#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include <shared/pluginservicehelper.h>
#include "propertycontroller.h"
#include "collectioncontroller.h"

#include "plugincontroller.h"

#define __FUNCTION__ __FUNCTION__

PluginController::PluginController (PropertyController* propertycontroller, CollectionController* collectioncontroller) {
    // add this class to plugins
    {
        PluginInfo* plugininfo = new PluginInfo(this);
        m_plugins.insert(plugininfo->plugin->pluginid(), plugininfo);
    }
    {
        PluginInfo* plugininfo = new PluginInfo(propertycontroller);
        m_plugins.insert(plugininfo->plugin->pluginid(), plugininfo);
    }
    {
        PluginInfo* plugininfo = new PluginInfo(collectioncontroller);
        m_plugins.insert(plugininfo->plugin->pluginid(), plugininfo);
    }

    const QDir plugindir = setup::pluginDir();

    AbstractPlugin *plugin;

    QStringList pluginfiles = plugindir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    for (int i=0;i<pluginfiles.size();++i) {
        const QString filename = plugindir.absoluteFilePath ( pluginfiles[i] );
        QPluginLoader* loader = new QPluginLoader ( filename, this );
        loader->setLoadHints(QLibrary::ResolveAllSymbolsHint);
        if (!loader->load()) {
            qWarning() << "Failed loading Plugin" << pluginfiles[i] << loader->errorString();
            delete loader;
            continue;
        }

        plugin = dynamic_cast<AbstractPlugin*> ( loader->instance() );
        if (!plugin) {
            qWarning() << "Failed to get instance" << filename;
            delete loader;
            continue;
        }

        const QString plugin_id = plugin->pluginid();
        PluginInfo* plugininfo = new PluginInfo(plugin);
        m_plugins.insert ( plugin_id, plugininfo );

        plugin->connectToServer(collectioncontroller, propertycontroller);
    }

    if (pluginfiles.empty())
        qWarning() << "No plugins found in" << plugindir;
}

PluginController::~PluginController()
{
    qDeleteAll(m_plugins);
}

void PluginController::initializePlugins() {
    QMap<QString,PluginInfo*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        (*i)->plugin->initialize();
    }
}

QMap< QString, PluginInfo* >::iterator PluginController::getPluginIterator() {
    return m_plugins.begin();
}

AbstractPlugin* PluginController::nextPlugin(QMap<QString,PluginInfo*>::iterator& index) {
    if (m_plugins.end()==index) return 0;
    return (*(index++))->plugin;
}

AbstractPlugin_settings* PluginController::nextSettingsPlugin(QMap<QString,PluginInfo*>::iterator& index) {
    while (m_plugins.end()!=index) {
        AbstractPlugin_settings* s = dynamic_cast<AbstractPlugin_settings*>((*(index++))->plugin);
        if (s) return s;
    }
    return 0;
}

AbstractPlugin_services* PluginController::nextServicePlugin(QMap<QString,PluginInfo*>::iterator& index) {
    while (m_plugins.end()!=index) {
        AbstractPlugin_services* s = dynamic_cast<AbstractPlugin_services*>((*(index++))->plugin);
        if (s) return s;
    }
    return 0;
}

AbstractPlugin_sessions* PluginController::nextSessionPlugin(QMap<QString,PluginInfo*>::iterator& index) {
    while (m_plugins.end()!=index) {
        AbstractPlugin_sessions* s = dynamic_cast<AbstractPlugin_sessions*>((*(index++))->plugin);
        if (s) return s;
    }
    return 0;
}

AbstractPlugin* PluginController::getPlugin(const QString& pluginid) {
    PluginInfo* pinfo = m_plugins.value(pluginid);
    if (!pinfo) return 0;
    return pinfo->plugin;
}

QList< QVariantMap > PluginController::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    QStringList pluginlist;
    QMap<QString,PluginInfo*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        pluginlist += (*i)->plugin->pluginid();
    }
    ServiceCreation s = ServiceCreation::createNotification("PluginController", "plugins");
    s.setData("plugins", pluginlist);
    l.append(s.getData());
    return l;
}

void PluginController::couchDB_Event_add(const QString& id, const QVariantMap& event_data) {
    QString destination_collectionuid = ServiceID::collectionid ( event_data );
    if ( destination_collectionuid.isEmpty() ) {
        qWarning() <<"Cannot register event. No collection set:"<<ServiceID::pluginid ( event_data ) << id;
        return;
    }

    AbstractPlugin* plugin = getPlugin ( ServiceID::pluginid ( event_data ) );
    AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*> ( plugin );
    if ( !executeplugin ) {
        qWarning() <<"Cannot register event. No plugin found:"<<ServiceID::pluginid ( event_data ) << id;
        return;
    }

    qDebug() << "register event:" << id << ServiceID::pluginid ( event_data ) << ServiceID::pluginmember ( event_data );
    executeplugin->unregister_event ( id, -1 );
    executeplugin->register_event ( event_data, destination_collectionuid, -1 );
    m_registeredevents.insert(id, executeplugin);
}

void PluginController::couchDB_Event_remove(const QString& id) {
    AbstractPlugin_services* executeplugin = m_registeredevents.take ( id );
    if ( executeplugin ) {
        qDebug() << "unregister event" << id << executeplugin;
        executeplugin->unregister_event ( id, -1 );
    }
}

