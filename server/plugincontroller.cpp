#include "plugincontroller.h"
#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include "servicecontroller.h"
#include <shared/pluginservicehelper.h>

#define __FUNCTION__ __FUNCTION__

PluginController::PluginController (ServiceController* servicecontroller) {
    servicecontroller->setPluginController(this);

    // add this class to plugins
    {
        PluginInfo* plugininfo = new PluginInfo(this);
        m_plugins.insert(pluginid(), plugininfo);
    }

    const QDir plugindir = pluginDir();

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

        plugin->connectToServer(servicecontroller);
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

QList< QVariantMap > PluginController::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    QStringList pluginlist;
    QMap<QString,PluginInfo*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        pluginlist += (*i)->plugin->pluginid();
    }
    ServiceCreation s = ServiceCreation::createNotification(PLUGIN_ID, "plugins");
    s.setData("plugins", pluginlist);
    l.append(s.getData());
    return l;
}
