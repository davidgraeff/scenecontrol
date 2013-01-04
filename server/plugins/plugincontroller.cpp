#include "plugins/plugincontroller.h"
#include "plugins/pluginprocess.h"
#include <scenecontrol.server/serverprovidedfunctions.h>
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

StorageNotifierConfiguration::StorageNotifierConfiguration(PluginController* pluginController) : mPluginController(pluginController)
{
	
}

void StorageNotifierConfiguration::documentChanged(const QString& /*filename*/, SceneDocument* /*oldDoc*/, SceneDocument* newDoc)
{
	if (newDoc->isType(SceneDocument::TypeConfiguration)) {
		mPluginController->startPluginProcessByConfiguration(newDoc);
	}
}

void StorageNotifierConfiguration::documentRemoved(const QString& /*filename*/, SceneDocument* document)
{
	if (document->isType(SceneDocument::TypeConfiguration)) {
		mPluginController->removePluginInstance(document->componentID(),document->instanceID());
	}
}

static PluginController* plugincontroller_instance = 0;
static int plugincontroller_nonew = 0;

PluginController* PluginController::instance() {
	if (!plugincontroller_instance && !plugincontroller_nonew) {
        plugincontroller_instance = new PluginController();
    }
    return plugincontroller_instance;
}

PluginController::PluginController () : m_exitIfNoPluginProcess(false) {
	// Listen to configuration document changes
	mStorageNotifierConfiguration = new StorageNotifierConfiguration(this);
	DataStorage::instance()->registerNotifier(mStorageNotifierConfiguration);
	// Plugin communication socket
    connect(&m_comserver, SIGNAL(newConnection()), SLOT(newConnection()));
    const QString name = QLatin1String(LOCALSOCKETNAMESPACE)+QLatin1String(LOCALSOCKETNAMESPACE);
    m_comserver.removeServer(name);
    if (!m_comserver.listen(name)) {
		qWarning() << "Could not establish local socket at" << name;
	}
}

PluginController::~PluginController()
{
	delete mStorageNotifierConfiguration;
	
    waitForPluginsAndExit();
	
    // Delete all plugin processes
//     qDeleteAll(m_pluginprocesses);
	
	plugincontroller_nonew = 1;
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
    // Delete all plugin process objects
	
    m_plugins.clear();
	// We need a copy here because the original "m_pluginprocesses" will be modified while iterating
	QSet<PluginProcess*> copy = m_pluginprocesses;
	QSet<PluginProcess*>::iterator i2 = copy.begin();
	while (copy.end()!= i2) {
		PluginProcess* p = (*i2);
        if (p)
			p->shutdown();
        ++i2;
    }
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

            SceneDocument doc(jsonData);
            PluginProcess* plugin = getPlugin(doc.componentID(),doc.instanceID());
            if (!plugin) {
                qWarning()<<"Socket auth failed: " << jsonData;
                socket->deleteLater();
            } else {
				// Plugin process is valid now
				plugin->setSocket(socket);
				emit pluginInstanceLoaded(doc.componentID(),doc.instanceID());
			}
			
        } else {
            qWarning()<<"Socket tried to connect: No authentification" << socket->readAll();
            socket->deleteLater();
        }
    }
}

PluginProcess* PluginController::getPlugin(const QString& pluginID, const QString& instanceID) {
	PluginProcess* p = m_plugins.value(pluginID).value(instanceID);
	if (p)
		return p;
	return 0;
}

QList< PluginProcess* > PluginController::getPlugins(const QString& pluginID, const QString instanceID)
{
	PluginProcess* p = m_plugins.value(pluginID).value(instanceID);
	if (p)
		return QList<PluginProcess*>() << p;
	return m_plugins.value(pluginID).values();
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
	//m_pluginlist.append(QLatin1String("server"));
	
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
		QList<SceneDocument*> configurations = DataStorage::instance()->filteredDocuments(SceneDocument::TypeConfiguration, filter.getData());
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

	PluginProcess* p = getPlugin(configuration->componentID(),configuration->instanceID());
	if (p) {
		delete p;
		qDebug() << "Restart Process" << configuration->componentUniqueID();
	} else
		qDebug() << "Start Process" << configuration->componentUniqueID();
	
	p = new PluginProcess( this, configuration->componentID(), configuration->instanceID());
	m_pluginprocesses.insert( p );
	m_plugins[configuration->componentID()].insert(configuration->instanceID(),p);
	p->startProcess();
	p->configChanged(configuration->id(), configuration->getData());
}

void PluginController::requestAllProperties(int sessionid) {
	PluginController::iterator i = begin();
	while (!i.eof()) {
		PluginProcess* plugin (*i);
		plugin->requestProperties(sessionid);
		++i;
	}
}

void PluginController::requestProperty ( const SceneDocument& property, int sessionid ) {
	PluginProcess* plugin = getPlugin(property.componentID(),property.instanceID());
	if (plugin) {
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
    qDebug() << "Plugin finished" << process->getPluginid();
    // Exit if no plugin processes are loaded
    if (m_pluginprocesses.isEmpty() && m_exitIfNoPluginProcess)
        QCoreApplication::exit(0);
}

void PluginController::removePluginInstance(const QString& pluginID, const QString instanceID) {
	PluginProcess* p = getPlugin(pluginID, instanceID);
	if (p)
		p->deleteLater();
}

int PluginController::execute(const SceneDocument& data, const QByteArray responseID, QObject* responseCallbackObject)
{
	// Thread safe: Only one thread at a time may send data to a plugin process
	QMutexLocker locker(&mExecuteMutex);
	
	if (data.componentID()==QLatin1String("server")) {
		ServerProvidedFunctions::execute(data, responseID, responseCallbackObject, -1);
		return 1;
	}
	
	// Call the remote method of the plugin
	int validCalls = 0;
	QList<PluginProcess*> plugins = getPlugins(data.componentID(),data.instanceID());
	foreach(PluginProcess* plugin, plugins) {
		if (!plugin->isValid())
			continue;
		if (responseCallbackObject)
			connect(plugin, SIGNAL(qtSlotResponse(QVariant,QByteArray,QString,QString)), responseCallbackObject,
					SLOT(pluginResponse(QVariant,QByteArray,QString,QString)));
		plugin->callQtSlot ( data, responseID );
		++validCalls;
	}
	return validCalls;
}

int PluginController::execute(const SceneDocument& data, int sessionid)
{
	// Thread safe: Only one thread at a time may send data to a plugin process
	QMutexLocker locker(&mExecuteMutex);
	
	if (data.componentID()==QLatin1String("server")) {
		ServerProvidedFunctions::execute(data, QByteArray(), 0, sessionid);
		return 1;
	}
	
	// Call the remote method of the plugin
	int validCalls = 0;
	QList<PluginProcess*> plugins = getPlugins(data.componentID(),data.instanceID());
	foreach(PluginProcess* plugin, plugins) {
		if (!plugin->isValid())
			continue;
		
		plugin->callQtSlot ( data, QByteArray(), sessionid );
		++validCalls;
	}
	return validCalls;
}
