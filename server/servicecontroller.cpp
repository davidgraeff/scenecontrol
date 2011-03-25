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

#define __FUNCTION__ __FUNCTION__

ServiceController::ServiceController ()
{
}

ServiceController::~ServiceController()
{
    qDeleteAll(m_id_to_xml);
    qDeleteAll(m_valid_services);
    qDeleteAll(m_plugins);
}

void ServiceController::load(bool service_dir_watcher) {
    loadPlugins();

    qDebug() << "Start: Load service provider";
    // files in {home}/roomcontrol/services
    QDir dir = serviceDir();
    const QStringList tfiles = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    for (int i=0;i<tfiles.size();++i) {
        directoryChanged(dir.absoluteFilePath ( tfiles[i] ), true);
    }

    connect(&m_dirwatcher, SIGNAL(directoryChanged(QString)), SLOT(directoryChanged(QString)));
    if (service_dir_watcher) {
        qDebug() << "Start observing directory"<<serviceDir().absolutePath();
        m_dirwatcher.addPath(serviceDir().absolutePath());
    }

    emit dataReady();

    // stats
    qDebug() << "Available Services    :" << m_id_to_xml.size();
    qDebug() << "Loaded    Services    :" << m_valid_services.size();
}

void ServiceController::directoryChanged(QString file, bool loading) {
    if (!loading) {
        if (!QFile::exists(file)) {
            qDebug() << "File removed:"<<file;
            QStringList list = file.split(QLatin1String("."));
            if (list.size()<3) return;
            removeService(list[1]);
            return;
        }
        qDebug() << "File changed:"<<file;
    }

    QFile f ( file );
    f.open ( QIODevice::ReadOnly );
    if ( !f.isOpen() )
    {
        qWarning() << "Couldn't open file" << file;
        return;
    }

    bool ok = true;
    QVariantMap result = QJson::Parser().parse ( &f, &ok ).toMap();
    f.close();
    if ( !ok )
    {
        qWarning() << "Not a json file" << file;
        return;
    }

    changeService ( result );
}

void ServiceController::changeService ( QVariantMap& data )
{
    if (!validateService(data)) return;

    if (IS_TOBEEXECUTED()) {
        executeService(data);
        return;
    }

    if (IS_TOBEREMOVED()) {
        removeFromDisk(data);
        return;
    }

    const QString filename = serviceFilename(ID(), UNIQUEID());

    if ((!IS_COLLECTION() && ID().isEmpty()) || UNIQUEID().isEmpty()) {
        QFile::remove(filename);
        qWarning() << "Invalid service file detected and removed" << filename;
    }

    ServiceStruct* service = m_valid_services.value(UNIQUEID());
    if (!service) service = new ServiceStruct();

    service->data = data;
    service->plugin = dynamic_cast<AbstractPlugin_services*>(m_id_to_plugin.value(ID()));

    m_valid_services.insert(UNIQUEID(),service);

    emit dataSync(data);
}

bool ServiceController::validateService( QVariantMap& data )
{
    // check remove
    if (IS_TOBEREMOVED()) {
        return (ID().size() && UNIQUEID().size());
    }

    QDomNode* node = 0;

    // check service/property id. collections do not have ids
    if (ID().size()) {
        node = m_id_to_xml.value(ID());
    } else if (IS_COLLECTION()) {
        node = m_id_to_xml.value(QLatin1String("collection"));
    }

    if (!node) {
        qWarning()<< "Cannot verify"<<ID()<<"with unique id"<<UNIQUEID() <<": No description for id found!";
        return false;
    }

    // check uid and add one if not there
    if (!IS_TOBEEXECUTED() && UNIQUEID().isEmpty()) {
        // Generate unique ids amoung all existing ids
        QString nid;
        do {
            nid = QUuid::createUuid().toString().remove ( QLatin1Char ( '{' ) ).remove ( QLatin1Char ( '}' ) );
        } while (m_valid_services.contains ( nid ));
        SETUNIQUEID(nid);
    }

    // check type
    const QString type = node->nodeName();
    if (IS_ACTION() && type != QLatin1String("action")) return false;
    if (IS_EVENT() && type != QLatin1String("event")) return false;
    if (IS_CONDITION() && type != QLatin1String("condition")) return false;
    if (IS_COLLECTION() && type != QLatin1String("collection")) return false;

    // check id. collections do not have ids
    {
        QDomNamedNodeMap attr = node->attributes();
        const QString id = attr.namedItem(QLatin1String("id")).nodeName();
        if (!IS_COLLECTION() && id != ID()) return false;
    }

    // check if all xml child notes are also represented in data
    QDomNodeList childs = node->childNodes();
    for (int i=0;i<childs.count();++i) {
        QDomNode node = childs.item(i);
        QString type = node.nodeName();
        if (type == QLatin1String("string")) {
        } else if (type == QLatin1String("int")) {
        } else if (type == QLatin1String("uint")) {
        } else if (type == QLatin1String("double")) {
        } else if (type == QLatin1String("enum")) {
        } else if (type == QLatin1String("time")) {
        } else if (type == QLatin1String("date")) {
        } else if (type == QLatin1String("url")) {
        } else if (type == QLatin1String("bool")) {
        } else {
            qWarning() << "Cannot verify"<<ID()<<"with unique id"<<UNIQUEID() <<": Unknown type";
            return false;
        }
        // check child id
		QDomNamedNodeMap attr = node.attributes();
        const QString id = attr.namedItem(QLatin1String("id")).nodeName();
        if (!data.contains(id)) {
			qWarning() << "Cannot verify"<<ID()<<"with unique id"<<UNIQUEID() <<": Entry"<<id;
			return false;
		}
    }
    return true;
}

void ServiceController::removeFromDisk ( const QVariantMap& data )
{
    const QString filename = serviceFilename ( ID(), UNIQUEID() );
    if ( !QFile::remove ( filename ) || QFileInfo(filename).exists() ) {
        qWarning() << "Couldn't remove file" << filename;
        return;
    }

    // collection: remove all childs
    if (IS_COLLECTION()) {
        QSet<QString> uids;
        QVariantMap actions = MAP("actions");
        foreach (QVariant v, actions) uids.insert(v.toString());
        QVariantList events = LIST("events");
        foreach (QVariant v, actions) uids.insert(v.toString());
        boolstuff::BoolExprParser parser;
        try {
            boolstuff::BoolExpr<std::string>* conditions = parser.parse(DATA("conditions").toStdString());
            std::set<std::string> vars;
            conditions->getTreeVariables(vars, vars);
            foreach (std::string v, vars) uids.insert(QString::fromStdString(v));
        } catch (boolstuff::BoolExprParser::Error) {
        }

        foreach (QString uid, uids) {
            ServiceStruct* s = service(uid);
            if (!s) continue;
            removeFromDisk(s->data);
        }
    }

    ServiceStruct* service = m_valid_services.take(UNIQUEID());
    delete service;
    if (service)
        emit dataSync(data, true);
}

void ServiceController::saveToDisk ( const QVariantMap& data )
{
    if ( IS_TOBEEXECUTED() )
    {
        qWarning() << "Requested to save an immediately-to-execute action!";
        return;
    }
    if ( IS_TOBEREMOVED() )
    {
        qWarning() << "Requested to save an about-to-be-removed action!";
        return;
    }

    const QString path = serviceFilename ( ID(), UNIQUEID() );

    QFile file ( path );
    if ( !file.open ( QIODevice::ReadWrite | QIODevice::Truncate ) )
    {
        qWarning() << __FUNCTION__ << "Couldn't save to " << path;
        return;
    }

    const QByteArray json = QJson::Serializer().serialize ( data );
    if ( data.isEmpty() )
    {
        qWarning() << __FUNCTION__ << "Saving failed to " << path;
    }
    else
    {
        file.write ( json );
    }
    file.close();
}

QString ServiceController::serviceFilename ( const QString& id, const QString& uid )
{
    return serviceDir().absoluteFilePath ( id + QLatin1String(".") + uid );
}

void ServiceController::event_triggered(const QString& event_id, const char* pluginid) {
    Q_UNUSED(pluginid);
    emit eventTriggered(event_id);
}

void ServiceController::execute_action(const QVariantMap& data, const char* pluginid) {
    Q_UNUSED(pluginid);
    executeService(data);
}

void ServiceController::property_changed(const QVariantMap& data, const QString& sessionid, const char* pluginid) {
    Q_UNUSED(pluginid);
    emit dataSync(data, false, sessionid);
    QSet<QString> plugins = m_propertyid_to_plugins.value(ID());
    foreach(QString plugin_id, plugins) {
        AbstractPlugin_otherproperties* plugin = m_plugin_otherproperties.value(plugin_id);
        if (plugin) plugin->otherPropertyChanged(data, sessionid);
    }
}

void ServiceController::register_listener(const QString& unqiue_property_id, const char* pluginid) {
    m_propertyid_to_plugins[unqiue_property_id].insert(QString::fromAscii(pluginid));
}

void ServiceController::unregister_all_listeners(const char* pluginid) {
    const QString id = QString::fromAscii(pluginid);
    QMutableMapIterator<QString, QSet<QString> > it(m_propertyid_to_plugins);
    while (it.hasNext()) {
        it.value().remove(id);
        if (it.value().isEmpty())
            it.remove();
    }
}

void ServiceController::unregister_listener(const QString& unqiue_property_id, const char* pluginid) {
    m_propertyid_to_plugins[unqiue_property_id].remove(QString::fromAscii(pluginid));
    if (m_propertyid_to_plugins[unqiue_property_id].isEmpty())
        m_propertyid_to_plugins.remove(unqiue_property_id);
}

void ServiceController::loadPlugins() {
    const QDir plugindir = pluginDir();
    QDir xmldir = plugindir;
    xmldir.cd(QLatin1String("xml"));

    AbstractPlugin *plugin;

    QStringList pluginfiles = plugindir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    for (int i=0;i<pluginfiles.size();++i) {
        const QString filename = plugindir.absoluteFilePath ( pluginfiles[i] );
        QPluginLoader* loader = new QPluginLoader ( filename, this );
        loader->setLoadHints(QLibrary::ResolveAllSymbolsHint);
        if (!loader->load()) {
            qWarning() << "Start: Failed loading" << filename << loader->errorString();
            delete loader;
            continue;
        }
        plugin = dynamic_cast<AbstractPlugin*> ( loader->instance() );
        if (!plugin) {
            qWarning() << "Start: Failed to get instance" << filename;
            delete loader;
            continue;
        }

        const QString pluginid = plugin->pluginid();
        qDebug() << "Start: Load Plugin"<<plugin->pluginid();
        PluginInfo* plugininfo = new PluginInfo(plugin);

        loadXML(xmldir.absoluteFilePath ( pluginid + QLatin1String(".xml") ));

        if (dynamic_cast<AbstractPlugin_services*>(plugin))
            m_plugin_services.insert(pluginid, (AbstractPlugin_services*)plugin);
        if (dynamic_cast<AbstractPlugin_otherproperties*>(plugin))
            m_plugin_otherproperties.insert(pluginid, (AbstractPlugin_otherproperties*)plugin);
        if (dynamic_cast<AbstractPlugin_settings*>(plugin))
            m_plugin_settings.insert(pluginid, (AbstractPlugin_settings*)plugin);

        plugin->connectToServer(this);

        m_plugins.append ( plugininfo );
    }

    if (pluginfiles.empty())
        qDebug() << "No plugins found in" << plugindir;

// load server xml
    loadXML(xmldir.absoluteFilePath(QLatin1String("server.xml")));

// initialize plugins
    foreach(PluginInfo* p, m_plugins) {
        p->plugin->initialize();
    }
}

void ServiceController::loadXML(const QString& filename) {
    // plugin namen muss für jeden unterpunkt rausgefunden werden
    // plugin namen muss zu enthaltenen ids per m_id_to_plugin map-bar sein
}

const QMap< QString, ServiceController::ServiceStruct* >& ServiceController::valid_services() const {
    return m_valid_services;
}

void ServiceController::useServerObject(AbstractPlugin* object) {
    const QString pluginid = object->pluginid();

    if (dynamic_cast<AbstractPlugin_services*>(object))
        m_plugin_services.insert(pluginid, (AbstractPlugin_services*)object);
    if (dynamic_cast<AbstractPlugin_otherproperties*>(object))
        m_plugin_otherproperties.insert(pluginid, (AbstractPlugin_otherproperties*)object);
    if (dynamic_cast<AbstractPlugin_settings*>(object))
        m_plugin_settings.insert(pluginid, (AbstractPlugin_settings*)object);
}

void ServiceController::removeServerObject(AbstractPlugin* object) {
    const QString pluginid = object->pluginid();

    // aus services entfernen
    {
        QMutableMapIterator<QString, ServiceStruct* > it(m_valid_services);
        while (it.hasNext()) {
            AbstractPlugin* p = dynamic_cast<AbstractPlugin*>(it.value()->plugin);
            if (p && p->pluginid() == pluginid) {
                it.remove();
            }
        }
    }

    // aus id mapping entfernen
    {
        QMutableMapIterator<QString, AbstractPlugin* > it(m_id_to_plugin);
        while (it.hasNext()) {
            if (it.value()->pluginid() == pluginid) {
                it.remove();
            }
        }
    }

    m_plugin_services.remove(pluginid);
    m_plugin_otherproperties.remove(pluginid);
    m_plugin_settings.remove(pluginid);

}

void ServiceController::removeService(const QString& uid) {
	ServiceStruct* service = m_valid_services.value(uid);
	if (!service) return;
	removeFromDisk(service->data);
}

void ServiceController::executeService(const QVariantMap& data) {
	if (!IS_TOBEEXECUTED() || !validateService((QVariantMap&)data)) return;
	AbstractPlugin* plugin = m_id_to_plugin.value(ID());
	AbstractPlugin_services* executeplugin = dynamic_cast<AbstractPlugin_services*>(plugin);
	if (!executeplugin) {
		qWarning()<<"Cannot execute service. No plugin found:"<<data;
		return;
	}
	executeplugin->execute(data);
}

void ServiceController::executeService(const QString& uid) {
	ServiceStruct* service = m_valid_services.value(uid);
	if (!service || !service->plugin) return;
	service->plugin->execute(service->data);
}
