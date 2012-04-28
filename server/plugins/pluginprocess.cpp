#include "pluginprocess.h"
#include "plugincontroller.h"
#include <QTimer>
#include "paths.h"
#include "config.h"
#include "plugins/plugincontroller.h"
#include "libdatabase/servicedata.h"
#include "libdatabase/database.h"
#include "socket.h"
#include "execute/collectioncontroller.h"
#include "paths.h"

#include <QElapsedTimer>
#include <QThread>
#include <QTimer>
#include <QDir>
#include <libdatabase/database_install.h>
#include <QCoreApplication>

PluginProcess::PluginProcess(PluginController* controller, const QString& pluginid, const QString& instanceid)
        : m_controller(controller), m_pluginCommunication(0), m_pluginid(pluginid), m_instanceid(instanceid) {
}

PluginProcess::~PluginProcess() {
	shutdown();
}

void PluginProcess::startProcess()
{
    const QDir plugindir = setup::pluginDir();
    QStringList files = plugindir.entryList(QStringList() << QString(QLatin1String("%1_plugin*")).arg(m_pluginid));
    if (files.size()!=1) {
        qWarning() << "Plugin executable not found!" << files;
        return;
    }
    m_filename = plugindir.absoluteFilePath(files[0]);
    m_pid = 0;
    if (!QProcess::startDetached(m_filename, QStringList() << m_instanceid, QString(), &m_pid))
        qWarning() << "Failed starting plugin process" << m_filename;

    connect(Database::instance(), SIGNAL(stateChanged()), SLOT(databaseStateChanged()));

    // Waiting at most 7 seconds for a respond otherwise kill the connection
	connect(&m_timeout, SIGNAL(timeout()), SLOT(responseTimeout()));
	m_timeout.setSingleShot(true);
    m_timeout.start(7000);
}

void PluginProcess::shutdown()
{
	m_timeout.stop();
    if (m_pid) {
        if (kill(m_pid, SIGTERM)==1)
            kill(m_pid, SIGKILL);
        m_pid = 0;
    }
    if (m_pluginCommunication) {
		m_pluginCommunication->disconnect(); // disconnect signal
        m_pluginCommunication->disconnectFromServer();
        m_pluginCommunication->deleteLater();
		m_pluginCommunication = 0;
    }
    m_controller->processFinished(this);
}

void PluginProcess::responseTimeout() {
    if (isValid())
        return;
    qWarning() << "Server: Plugin process timeout" << m_filename << m_pid;
    shutdown();
}

void PluginProcess::communicationSocketStateChanged() {
    if (!m_pluginCommunication || m_pluginCommunication->state()!=QLocalSocket::UnconnectedState)
        return;
    shutdown();
}

void PluginProcess::databaseStateChanged() {
    // Process not started or no local socket: abort
    if (!isValid())
        return;

    Database* b = Database::instance();
    if (b->state()==Database::ConnectedState) {
        b->requestEvents(m_pluginid, m_instanceid);
    } else if (b->state()==Database::DisconnectedState) {
        clear();
    }
}

void PluginProcess::setSocket(QLocalSocket* socket)
{
    if (m_pluginCommunication) {
        m_pluginCommunication->disconnect();
        delete m_pluginCommunication;
    }
    m_pluginCommunication = socket;
    qRegisterMetaType<QLocalSocket::LocalSocketState>("QLocalSocket::LocalSocketState");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    connect(m_pluginCommunication, SIGNAL(readyRead()), SLOT(readyReadPluginData()));
    connect(m_pluginCommunication, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), SLOT(communicationSocketStateChanged()), Qt::DirectConnection);

    // initialize plugin
    initialize();

	// cached configurations
	QMap<QByteArray, QVariantMap>::iterator i = m_configcache.begin();
	for(;i!=m_configcache.end();++i)
		configChanged(i.key(),i.value());
	m_configcache.clear();
	
    // now try to fetch events from database
    databaseStateChanged();
}

QLocalSocket* PluginProcess::getSocket() {
    return m_pluginCommunication;
}

bool PluginProcess::isValid()
{
    return m_pluginCommunication;
}

QString PluginProcess::getPluginid() {
    return m_pluginid;
}

QString PluginProcess::getInstanceid() {
    return m_instanceid;
}

bool PluginProcess::writeToPlugin(const QVariantMap& data) {
    // check state. Sometimes write calls are one after each other and the
    // event system does not get a chance to check the socket state in between
    if (m_pluginCommunication->state()!=QLocalSocket::ConnectedState) {
        communicationSocketStateChanged();
        return false;
    }
    // write data
    QDataStream streamout(m_pluginCommunication);
    streamout << data;
    streamout.writeRawData("\n\t\0", 3);
    return true;
}

void PluginProcess::readyReadPluginData()
{
    m_chunk.append(m_pluginCommunication->readAll());

    int indexOfChunkEnd;
    while ((indexOfChunkEnd = m_chunk.indexOf("\n\t")) != -1) {
        // Read data and decode into a QVariantMap; clear chunk buffer
        QVariantMap variantdata;
        {
            const QByteArray r(m_chunk, indexOfChunkEnd);
            QDataStream stream(r);
            stream >> variantdata;
        }
        // Drop chunk and chunk-complete-bytes
        m_chunk.remove(0,indexOfChunkEnd+3);

        const QByteArray method = ServiceData::method(variantdata);
        //qDebug() << "Data from" << id << variantdata;
        if (method == "methodresponse") {
            emit qtSlotResponse(variantdata.value(QLatin1String("response_")),
                                variantdata.value(QLatin1String("responseid_")).toByteArray(), m_pluginid, m_instanceid);
        } else if (method == "changeConfig") {
            const QByteArray configcategory = ServiceData::configurationkey(variantdata).toAscii();
            if (configcategory.isEmpty()) {
                qWarning() << "Server: Request changeConfig for" << m_pluginid << m_instanceid <<"but no key provided";
                continue;
            }
            // store new configuration value in database
            Database::instance()->changePluginConfiguration(m_pluginid, m_instanceid, configcategory, variantdata);
        } else if (method == "changeProperty") {
            // Get session id and remove id from QVariantMap
            const int sessionid = ServiceData::sessionid(variantdata);
            ServiceData::removeSessionID(variantdata);
            // propagate changed property
            Socket::instance()->propagateProperty(variantdata, sessionid);
        } else if (method == "eventTriggered") {
            const QString collectionid = ServiceData::collectionid(variantdata);
            if (collectionid.isEmpty()) {
                qWarning() << "Server: Request collection execution by event for" << m_pluginid << m_instanceid <<"but no collectionid provided";
                continue;
            }
            //qDebug() << "eventTriggered";
            CollectionController::instance()->requestExecutionByCollectionId(collectionid);
        } else {
            qWarning() << "Unknown data from plugin" << m_chunk;
        }
    }
}

void PluginProcess::initialize() {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "initialize");
    writeToPlugin(data);
}

void PluginProcess::clear() {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "clear");
    writeToPlugin(data);
}

void PluginProcess::configChanged(const QByteArray& configid, const QVariantMap& data) {
    if (data.isEmpty()) {
        qWarning() << "configChanged data empty!";
        return;
    }
    if (!m_pluginCommunication) {
		m_configcache.insert(configid, data);
        return;
	}
    QVariantMap mdata(data);
    ServiceData::setMethod(mdata, "configChanged");
    ServiceData::setConfigurationkey(mdata, configid);
    writeToPlugin(mdata);
}

void PluginProcess::requestProperties(int sessionid) {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "requestProperties");
    ServiceData::setSessionID(data, sessionid);
    writeToPlugin(data);
}

void PluginProcess::unregister_event(const QString& eventid) {
    if (!m_pluginCommunication)
        return;
    if (eventid.isEmpty()) {
        qWarning() << "unregister_event eventid empty!";
        return;
    }
    QVariantMap mdata;
    ServiceData::setMethod(mdata, "unregister_event");
    mdata[QLatin1String("eventid")] = eventid;
    writeToPlugin(mdata);
}

void PluginProcess::session_change(int sessionid, bool running)
{
    if (!m_pluginCommunication)
        return;
    QVariantMap mdata;
    ServiceData::setMethod(mdata, "session_change");
    ServiceData::setSessionID(mdata, sessionid);
    mdata[QLatin1String("running")] = running;
    writeToPlugin(mdata);
}

void PluginProcess::callQtSlot(const QVariantMap& methodAndArguments, const QByteArray& responseid, int sessionid) {
    if (!ServiceData::hasMethod(methodAndArguments)) {
        qWarning() << "Call of qt slot without method" << methodAndArguments;
        return;
    }
    QVariantMap modified = methodAndArguments;
    modified[QLatin1String("responseid_")] = responseid;
    ServiceData::setSessionID(modified, sessionid);
    writeToPlugin(modified);
}