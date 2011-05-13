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

#define __FUNCTION__ __FUNCTION__

PluginController::PluginController (ServiceController* servicecontroller) {
    servicecontroller->setPluginController(this);

    const QDir plugindir = pluginDir();

    AbstractPlugin *plugin;

    // server objects are already registered. try to load the corresponding xml files
    QMap<QString,PluginInfo*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        loadXML(xmlFile((*i)->plugin->pluginid()));
    }

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

        const QString pluginid = plugin->pluginid();
        PluginInfo* plugininfo = new PluginInfo(plugin);
        m_plugins.insert ( pluginid, plugininfo );

        loadXML(xmlFile(pluginid));

        plugin->connectToServer(servicecontroller);
    }

    if (pluginfiles.empty())
        qWarning() << "No plugins found in" << plugindir;
}

PluginController::~PluginController()
{
    qDeleteAll(m_id_to_xml);
    qDeleteAll(m_plugins);
}

void PluginController::loadXML(const QString& filename) {
    QDomDocument doc;
    QFile file(filename);
    QString errormsg;
    int line = 0;
    int column = 0;
    if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file, &errormsg, &line, &column)) {
        file.close();
        qWarning() << "Failed loading plugin xml"<<filename<<errormsg<<line<<column;
        return;
    }
    file.close();

    // find plugin node
    AbstractPlugin* plugin = 0;
    QDomNode node;
    QDomNodeList childs = doc.childNodes();
    for (int i=0;i<childs.size();++i) {
        node = childs.item(i);
        if (!node.isElement()) continue;
        if (node.nodeName()!=QLatin1String("plugin"))
            continue;
    }

    // get id and version attributes
    const QString pluginid = node.attributes().namedItem(QLatin1String("id")).nodeValue();
    const QString version = node.attributes().namedItem(QLatin1String("version")).nodeValue();

    PluginInfo* pinfo = m_plugins.value(pluginid);
    if (pinfo) {
        pinfo->setVersion(version);
        plugin = pinfo->plugin;
    }

    if (!plugin) {
        qWarning()<<"No plugin for xml description found!" << filename;
        return;
    }

    // register all defined actions, events, conditions and properties
    QSet<QString> allowedElements;
    allowedElements << QLatin1String("action") << QLatin1String("event") << QLatin1String("condition") << QLatin1String("execute") << QLatin1String("notification") << QLatin1String("model");
    QDomNodeList items_of_plugin = node.childNodes();
    for (int j=0;j<items_of_plugin.size();++j) {
        QDomNode item = items_of_plugin.item(j);
        if (!item.isElement()) continue;
        // get id (for example "periodic_time_event")
        const QString nodeid = item.attributes().namedItem(QLatin1String("id")).nodeValue();
        if (QLatin1String("collection") == item.nodeName()) {
            if (m_id_to_xml.contains(QLatin1String("collection"))) {
                qWarning()<<"Multiple xml definition of" << QLatin1String("collection");
                continue;
            }
            m_id_to_xml.insert(QLatin1String("collection"), new QDomNode(item.cloneNode()));
        } else {
            if (!allowedElements.contains(item.nodeName()) || nodeid.isEmpty()) continue;
            // plugin id + service id = global unique id
            const QString gid = pluginid + QLatin1String("_") + nodeid;

            if (m_id_to_xml.contains(gid)) {
                qWarning()<<"Multiple xml definition of" << gid;
                continue;
            }
            m_id_to_xml.insert(gid, new QDomNode(item.cloneNode()));
        }
    }
}
void PluginController::initializePlugins() {
    QMap<QString,PluginInfo*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        (*i)->plugin->initialize();
    }
}

int PluginController::knownServices() {
    return m_id_to_xml.size();
}


void PluginController::registerPluginFromObject(AbstractPlugin* object, ServiceController* servicecontroller) {
	object->connectToServer(servicecontroller);
    const QString pluginid = object->pluginid();
    PluginInfo* plugininfo = new PluginInfo(object);
    m_plugins.insert ( pluginid, plugininfo );
    loadXML(xmlFile(pluginid));
}

void PluginController::deregisterPluginFromObject(AbstractPlugin* object, ServiceController* servicecontroller) {
    const QString pluginid = object->pluginid();
    servicecontroller->removeServicesUsingPlugin(pluginid);
    m_plugins.remove(pluginid);
}

QMap< QString, PluginInfo* >::iterator PluginController::getPluginIterator() {
    return m_plugins.begin();
}

AbstractPlugin* PluginController::nextPlugin(QMap<QString,PluginInfo*>::iterator& index) {
    if (m_plugins.end()==index) return 0;
    return (*(index++))->plugin;
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

QDomNode* PluginController::getPluginDom(const QString& service_gid) {
    return m_id_to_xml.value(service_gid);
}
