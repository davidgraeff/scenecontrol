#include "plugins/plugincontroller.h"
#include "plugins/pluginprocess.h"
#include "shared/jsondocuments/scenedocument.h"
#include "shared/jsondocuments/json.h"
#include "shared/utils/paths.h"
#include "libdatastorage/datastorage.h"
#include "libdatastorage/importexport.h"

#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <QElapsedTimer>

#define __FUNCTION__ __FUNCTION__
#define LOCALSOCKETNAMESPACE "sceneserver"

static PluginController* plugincontroller_instance = 0;

PluginController* PluginController::instance() {
    if (!plugincontroller_instance) {
        plugincontroller_instance = new PluginController();
    }
    return plugincontroller_instance;
}

PluginController::PluginController () : m_exitIfNoPluginProcess(false) {
    connect(&m_comserver, SIGNAL(newConnection()), SLOT(newConnection()));
    const QString name = QLatin1String(LOCALSOCKETNAMESPACE)+QLatin1String(LOCALSOCKETNAMESPACE);
    m_comserver.removeServer(name);
    if (!m_comserver.listen(name)) {
		qWarning() << "Could not establish local socket at" << name;
	}
}

PluginController::~PluginController()
{
    waitForPluginsAndExit();
    // Delete all plugin processes
    qDeleteAll(m_pluginprocesses);
}

bool PluginController::valid()
{
    return m_comserver.isListening();
}

void PluginController::waitForPluginsAndExit()
{
    // Already in the state of exiting
    if (m_plugins.isEmpty() && m_exitIfNoPluginProcess)
        return;

    qDebug() << "Shutdown..." << m_pluginprocesses.size() << "Plugin processes";
    m_exitIfNoPluginProcess = true;
    // Unregister all events
    QMap<QString,PluginProcess*>::iterator i = m_registeredevents.begin();
    for (;i!=m_registeredevents.end();++i) {
        PluginProcess* executeplugin = i.value();
        executeplugin->unregister_event ( i.key() );
    }
    m_registeredevents.clear();
    // Delete all plugin process objects
	foreach(PluginProcess* p, m_plugins) {
		p->shutdown();
	}
    m_plugins.clear();
    QSet<PluginProcess*>::iterator i2 = m_pluginprocesses.begin();
    while (m_pluginprocesses.end()!= i2) {
        (*i2)->shutdown();
        ++i2;
    }

    // Exit if no plugin processes are loaded
    if (m_pluginprocesses.isEmpty())
        QCoreApplication::exit(0);
}

void PluginController::newConnection()
{
    while ( QLocalSocket* socket = m_comserver.nextPendingConnection()) {
        QElapsedTimer t;
        t.start();
        while (!t.hasExpired(2000) && socket->peek(300).indexOf("\n\t") == -1) {
            QCoreApplication::processEvents();
        }

        int index = socket->peek(300).indexOf("\n\t");
        if (index!=-1) {
            QByteArray allinput = socket->read(index);
            // throw away rest of input
            socket->readAll();
            // Read data and decode into a QVariantMap; clear chunk buffer
            QVariantMap jsonData;
            {
                QDataStream stream(allinput);
                stream >> jsonData;
            }

            PluginProcess* plugin = getPlugin(SceneDocument(jsonData).componentUniqueID());
            if (!plugin) {
                qWarning()<<"Socket auth failed: " << jsonData;
                socket->deleteLater();
            } else
                plugin->setSocket(socket);
        } else {
            qWarning()<<"Socket tried to connect: No authentification" << socket->readAll();
            socket->deleteLater();
        }
    }
}

QMap< QString, PluginProcess* >::iterator PluginController::getPluginIterator() {
    return m_plugins.begin();
}

PluginProcess* PluginController::nextPlugin(QMap< QString, PluginProcess* >::iterator& index) {
    if (m_plugins.end()==index) return 0;
    return (*(index++));
}

PluginProcess* PluginController::getPlugin(const QString& pluginUid) {
    return m_plugins.value(pluginUid);
}

void PluginController::doc_changed(const SceneDocument* doc) {
	if (doc->checkType(SceneDocument::TypeEvent)) {
		PluginProcess* plugin = getPlugin ( doc->componentUniqueID());
		if ( !plugin ) {
			qWarning() <<"Plugins: Cannot register event. No plugin found:"<< doc->componentID() << doc->id();
			return;
		}

		plugin->unregister_event ( doc->id() );
		plugin->callQtSlot(*doc);
		m_registeredevents.insert(doc->id(), plugin);
	} else if (doc->checkType(SceneDocument::TypeConfiguration)) {
		startPluginProcessByConfiguration(doc);
	}
}

void PluginController::doc_removed(const SceneDocument* doc) {
	if (doc->checkType(SceneDocument::TypeEvent)) {
		PluginProcess* executeplugin = m_registeredevents.take ( doc->id() );
		if ( executeplugin ) {
	//         qDebug() << "Plugins: unregister event" << id << executeplugin;
			executeplugin->unregister_event ( doc->id() );
		}
	} else if (doc->checkType(SceneDocument::TypeConfiguration)) {
		PluginProcess* p = getPlugin(doc->componentUniqueID());
		if (p)
			p->deleteLater();
	}
}

void PluginController::scanPlugins() {
    const QDir plugindir = setup::pluginDir();
    QStringList pluginfiles = plugindir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    if (pluginfiles.empty()) {
        qWarning() << "Plugins: No plugins found in" << plugindir.absolutePath();
        return;
    }

    bool importdirfound;
    QDir importdir(setup::dbimportDir(&importdirfound));

    qDebug() << "Plugin path:" << plugindir.absolutePath();

	if (!importdirfound)
        qWarning() << "Server: Datastorage initial import path not found!";
	else
		qDebug() << "Plugin initial configurations:" << importdir.absolutePath();

	m_pluginlist.clear();
	
    for (int i=0;i<pluginfiles.size();++i) {
        const QString componentid = pluginfiles[i].mid(0, pluginfiles[i].lastIndexOf(QLatin1String("_plugin")));
		m_pluginlist.append(componentid);

        // Install missing files for this plugin first
        if (importdirfound && importdir.cd( componentid )) {
			Datastorage::VerifyPluginDocument verifier( componentid );
			Datastorage::importFromJSON(*DataStorage::instance(), importdir.absolutePath(), false, &verifier);
			importdir.cdUp();
		}

		// Get all configurations of this plugin
		SceneDocument filter;
		filter.setComponentID( componentid );
		QList<SceneDocument*> configurations = DataStorage::instance()->requestAllOfType(SceneDocument::TypeConfiguration, filter.getData());
		for (int pi = 0; pi < configurations.size(); ++pi) {
			startPluginProcessByConfiguration(configurations[pi]);
		}
	}
}

void PluginController::startPluginProcessByConfiguration ( const SceneDocument* configuration )
{
	if (!configuration->hasComponentUniqueID()) {
		qWarning() << "Server: Document incomplete. PluginID or PluginInstance is missing!" << configuration->getData();
		return;
	}

	PluginProcess* p = getPlugin(configuration->componentUniqueID());
	if (p) {
		delete p;
		qDebug() << "Restart Process" << configuration->componentUniqueID();
	} else
		qDebug() << "Start Process" << configuration->componentUniqueID();
	
	p = new PluginProcess( this, configuration->componentID(), configuration->instanceID());
	m_pluginprocesses.insert( p );
	m_plugins.insert(configuration->componentUniqueID(),p);
	p->startProcess();
	p->configChanged(configuration->id(), configuration->getData());
}

void PluginController::requestAllProperties(int sessionid) {
	QMap<QString,PluginProcess*>::iterator i = getPluginIterator();
	while (PluginProcess* plugin = nextPlugin(i)) {
		plugin->requestProperties(sessionid);
	}
}

QStringList PluginController::pluginids() const
{
    return m_pluginlist;
}

void PluginController::processFinished(PluginProcess* process) {
    if (!m_pluginprocesses.remove(process))
		return;
	m_plugins.remove(process->getPluginid()+process->getInstanceid());
    process->deleteLater();
    // Remove all registered events of this plugin out of the map m_registeredevents
    // (they are not active already due to removing the plugin process instance but to conserve memory)
    QMutableMapIterator<QString, PluginProcess*> i = m_registeredevents;
    while (i.hasNext()) {
        i.next();
        if (i.value() == process)
            i.remove();
    }
    qDebug() << "Plugin finished" << process->getPluginid();
    // Exit if no plugin processes are loaded
    if (m_pluginprocesses.isEmpty() && m_exitIfNoPluginProcess)
        QCoreApplication::exit(0);
}
