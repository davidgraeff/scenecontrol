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
    QJson::Parser parser;
    // files in {home}/roomcontrol/services
    QDir dir = serviceDir();
    const QStringList tfiles = dir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    for (int i=0;i<tfiles.size();++i) {
        const QString file = dir.absoluteFilePath ( tfiles[i] );

        bool ok = true;
        QFile f ( file );
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

        changeService ( result );
    }

    //TODO dir watcher

    emit dataReady();

    // stats
    qDebug() << "Available Services    :" << m_id_to_xml.size();
    qDebug() << "Loaded    Services    :" << m_valid_services.size();
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
        SETID(nid);
    }

    // check type
    const QString type = node->nodeName();
    if (IS_ACTION() && type != QLatin1String("action")) return false;
    if (IS_EVENT() && type != QLatin1String("event")) return false;
    if (IS_CONDITION() && type != QLatin1String("condition")) return false;
    if (IS_COLLECTION() && type != QLatin1String("collection")) return false;

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
    }
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
    return m_savedir.absoluteFilePath ( id + uid );
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

    int offered_services = 0;
    AbstractPlugin *plugin;

    QStringList pluginfiles = plugindir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    for (int i=0;i<pluginfiles.size();++i)
        pluginfiles[i] = plugindir.absoluteFilePath ( pluginfiles[i] );
    if (pluginfiles.empty())
        qDebug() << "No plugins found in" << plugindir;

    foreach ( QString filename, pluginfiles )
    {
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

        //TODO XML name version
        const QString name;
        const QString version;
        //TODO andere Abstract Interfaces

        plugin->connectToServer(this);

        qDebug() << "Start: Load Plugin"<<plugin->pluginid();

        m_plugins.append (new PluginInfo(plugin, name, version) );
    }

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
