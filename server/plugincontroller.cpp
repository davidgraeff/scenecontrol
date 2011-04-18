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
    QDir xmldir = plugindir;
    xmldir.cd(QLatin1String("xml"));

    AbstractPlugin *plugin;

    // server objects are already registered. try to load the corresponding xml files
    for (int i=0;i<m_plugins.size();++i) {
        PluginInfo* p = m_plugins[i];
        loadXML(xmldir.absoluteFilePath(p->plugin->pluginid() + QLatin1String(".xml")));
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
        m_plugins.append ( plugininfo );

        loadXML(xmldir.absoluteFilePath ( pluginid + QLatin1String(".xml") ));
        qDebug() << "Loaded Plugin"<<pluginid<<plugininfo->version;

        plugin->connectToServer(servicecontroller);
    }

    if (pluginfiles.empty())
        qDebug() << "No plugins found in" << plugindir;
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

    // find the corresponding plugin
    for (int i=0;i<m_plugins.size();++i) {
        PluginInfo* p = m_plugins[i];
        if (p->plugin->pluginid() == pluginid) {
            p->setVersion(version);
            plugin = p->plugin;
            break;
        }
    }

    if (!plugin) {
        qWarning()<<"No plugin for xml description found!" << filename;
        return;
    }

    // register all defined actions, events, conditions and properties
    QDomNodeList items_of_plugin = node.childNodes();
    for (int j=0;j<items_of_plugin.size();++j) {
        QDomNode item = items_of_plugin.item(j);
        if (!item.isElement()) continue;
        // get id (for example "periodic_time_event")
        const QString nodeid = item.attributes().namedItem(QLatin1String("id")).nodeValue();
        const QString id = pluginid + QLatin1String("_") + nodeid;

        if (m_id_to_xml.contains(id)) {
            qWarning()<<"Multiple xml definition of" << id;
            continue;
        }

        /*        QString t;
                QTextStream s(&t);
        		item.save(s,3);
        		qDebug()<<t;*/

        m_id_to_xml.insert(id, new QDomNode(item.cloneNode()));
        m_id_to_plugin.insert(id, plugin);
    }
}
void PluginController::initializePlugins() {
    for (int i=0;i<m_plugins.size();++i) {
        m_plugins[i]->plugin->initialize();
    }
}
int PluginController::knownServices() {
    return m_id_to_xml.size();
}


void PluginController::registerPluginFromObject(AbstractPlugin* object) {
    const QString pluginid = object->pluginid();

    PluginInfo* plugininfo = new PluginInfo(object);
    m_plugins.append ( plugininfo );

}

void PluginController::deregisterPluginFromObject(AbstractPlugin* object, ServiceController* servicecontroller) {
    const QString pluginid = object->pluginid();
    servicecontroller->removeServicesUsingPlugin(pluginid);

    // aus id mapping entfernen
    {
        QMutableMapIterator<QString, AbstractPlugin* > it(m_id_to_plugin);
        while (it.hasNext()) {
            it.next();
            if (it.value()->pluginid() == pluginid) {
                it.remove();
            }
        }
    }

    // aus plugin liste entfernen
    {
        QMutableListIterator<PluginInfo*> it(m_plugins);
        while (it.hasNext()) {
            it.next();
            if (it.value()->plugin->pluginid() == pluginid) {
                it.remove();
            }
        }
    }
}

AbstractPlugin* PluginController::nextPlugin(int& index) {
    if (m_plugins.size()<=index) return 0;
    return m_plugins[index++]->plugin;
}

AbstractPlugin_services* PluginController::nextServicePlugin(int& index) {
    while (m_plugins.size()>index) {
        AbstractPlugin_services* s = dynamic_cast<AbstractPlugin_services*>(m_plugins[index++]->plugin);
        if (s) return s;
    }
    return 0;
}

AbstractPlugin* PluginController::getPlugin(const QString& serviceid) {
    return m_id_to_plugin.value(serviceid);
}

QDomNode* PluginController::getPluginDom(const QString& serviceid) {
    return m_id_to_xml.value(serviceid);
}
