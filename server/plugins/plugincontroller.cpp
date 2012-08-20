#include "plugins/plugincontroller.h"
#include "plugins/pluginprocess.h"
#include "execute/collectioncontroller.h"
#include "shared/jsondocuments/scenedocument.h"
#include "shared/jsondocuments/json.h"
#include "libdatastorage/datastorage.h"
#include "libdatastorage/importexport.h"
#include "socket.h"
#include "paths.h"

#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <QElapsedTimer>

#define __FUNCTION__ __FUNCTION__
#define MAGICSTRING "roomcontrol_"
#define COMSERVERSTRING "server"

static PluginController* plugincontroller_instance = 0;

PluginController* PluginController::instance() {
    if (!plugincontroller_instance) {
        plugincontroller_instance = new PluginController();
    }
    return plugincontroller_instance;
}

PluginController::PluginController () : m_exitIfNoPluginProcess(false) {
    connect(&m_comserver, SIGNAL(newConnection()), SLOT(newConnection()));
    const QString name = QLatin1String(MAGICSTRING) + QLatin1String(COMSERVERSTRING);
    m_comserver.removeServer(name);
    m_comserver.listen(name);
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
            

            PluginProcess* plugin = getPlugin(SceneDocument(jsonData).pluginuid());
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

void PluginController::doc_changed(const SceneDocument &doc) {
	if (!doc.checkType(SceneDocument::TypeEvent))
		return;
    PluginProcess* plugin = getPlugin ( doc.pluginuid());
    if ( !plugin ) {
        qWarning() <<"Plugins: Cannot register event. No plugin found:"<< doc.pluginid() << doc.id();
        return;
    }

    plugin->unregister_event ( doc.id() );
    plugin->callQtSlot(doc.getData());
    m_registeredevents.insert(doc.id(), plugin);
}

void PluginController::doc_removed(const SceneDocument &doc) {
	if (!doc.checkType(SceneDocument::TypeEvent))
		return;
    PluginProcess* executeplugin = m_registeredevents.take ( doc.id() );
    if ( executeplugin ) {
//         qDebug() << "Plugins: unregister event" << id << executeplugin;
        executeplugin->unregister_event ( doc.id() );
    }
}

void PluginController::scanPlugins() {
    const QDir plugindir = setup::pluginDir();
    QStringList pluginfiles = plugindir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    if (pluginfiles.empty()) {
        qWarning() << "Plugins: No plugins found in" << plugindir;
        return;
    }

    bool importdirfound;
    QDir importdir(setup::dbimportDir(&importdirfound));
    if (!importdirfound)
        qWarning() << "Server: Datastorage initial import path not found!";

    for (int i=0;i<pluginfiles.size();++i) {
        QString pluginid = QFileInfo(pluginfiles[i]).baseName();
        pluginid = pluginid.mid(0, pluginid.lastIndexOf(QLatin1String("_plugin")));

        // Install missing files for this plugin first
        if (importdirfound && importdir.cd(pluginid)) {
			Datastorage::VerifyPluginDocument verifier(pluginid);
			Datastorage::importFromJSON(*DataStorage::instance(), importdir.absolutePath(), false, &verifier);
			importdir.cdUp();
		}

		// Get all configurations of this plugin
		SceneDocument filter;
		filter.setPluginid(pluginid);
		QList<SceneDocument*> configurations = DataStorage::instance()->requestAllOfType(SceneDocument::TypeConfiguration, filter.getData());
		for (int pi = 0; pi < configurations.size(); ++pi) {
			SceneDocument* configuration = configurations[i];
			if (!configuration->hasPluginuid()) {
				qWarning() << "Server: Document incomplete. PluginID or PluginInstance is missing!" << configurations[i]->getData();
				continue;
			}

			PluginProcess* p = getPlugin(configuration->pluginuid());
			if (!p) {
				p = new PluginProcess( this, pluginid, configuration->plugininstance());
				m_pluginprocesses.insert( p );
				m_plugins.insert(configuration->pluginuid(),p);
				p->startProcess();
				qDebug() << "Process start" << pluginid << p->getInstanceid();
			}
			p->configChanged(configuration->configurationkey(), configuration->getData());
		}
	}
}

void PluginController::requestAllProperties(int sessionid) {
    {
        QMap<QString,PluginProcess*>::iterator i = getPluginIterator();
        while (PluginProcess* plugin = nextPlugin(i)) {
            plugin->requestProperties(sessionid);
        }
    }

    // Generate plugin list
    QStringList pluginlist;
    QMap<QString,PluginProcess*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        const QString identifier = (*i)->getPluginid() + QLatin1String(":") + (*i)->getInstanceid();
        pluginlist += identifier;
    }
    SceneDocument s = SceneDocument::createNotification("plugins");
    s.setData("plugins", pluginlist);
    s.setPluginid(QLatin1String("PluginController"));
    Socket::instance()->propagateProperty(s.getData(), sessionid);
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
