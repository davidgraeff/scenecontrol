#include "pluginprocess.h"
#include <shared/pluginservicehelper.h>
#include "plugincontroller.h"
#include "couchdb.h"
#include "socket.h"
#include "collectioncontroller.h"

PluginCommunication::PluginCommunication(PluginController* controller, QLocalSocket* socket) : m_controller(controller) {
    m_pluginCommunication=socket;
    connect(m_pluginCommunication, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(m_pluginCommunication, SIGNAL(disconnected()), SLOT(disconnected()));
    // Request plugin id
    QVariantMap data;
    ServiceData::setMethod(data, "pluginid");
    QDataStream stream(m_pluginCommunication);
    stream << data << '\n';
    // Waiting at most 7 seconds for a respond otherwise kill the connection
    QTimer::singleShot(7000, this, SLOT(startTimeout()));
    readyRead();
}

PluginCommunication::~PluginCommunication() {
    delete m_pluginCommunication;
}

void PluginCommunication::readyRead()
{
    while (m_pluginCommunication->canReadLine()) {
        const QByteArray l = m_pluginCommunication->readLine();
        QDataStream stream(l);
        QVariantMap variantdata;
        stream >> variantdata;
        const QByteArray method = ServiceData::method(variantdata);
        qDebug() << "Data from" << id << variantdata;
        if (method == "pluginid") {
            id = ServiceData::pluginid(variantdata);
            if (id.isEmpty()) {
                qWarning() << "Server: Plugin did not send pluginid!";
                m_controller->removePluginFromPending(this);
                return;
            }
            // Add to normal plugins
            m_controller->addPlugin(id, this);
            // request configuration
            CouchDB::instance()->requestPluginSettings(id);
        } else if (method == "changeConfig") {
            const QString key = ServiceData::configurationkey(variantdata);
            if (key.isEmpty()) {
                qWarning() << "Server: Request changeConfig for" << id <<"but no key provided";
                continue;
            }
            // store new configuration value in database
            CouchDB::instance()->changePluginConfiguration(id, key, variantdata);
        } else if (method == "changeProperty") {
            // Get session id and remove id from QVariantMap
            const int sessionid = ServiceData::sessionid(variantdata);
            ServiceData::setSessionID(variantdata, -1);
            // propagate changed property
            Socket::instance()->propagateProperty(variantdata, sessionid);
        } else if (method == "eventTriggered") {
            const QString collectionid = ServiceData::collectionid(variantdata);
            if (collectionid.isEmpty()) {
                qWarning() << "Server: Request collection execution by event for" << id <<"but no collectionid provided";
                continue;
            }
            CollectionController::instance()->requestExecutionByCollectionId(collectionid);
        }
    }
}

void PluginCommunication::setVersion(const QString& version) {
    m_version = version;
}

void PluginCommunication::initialize() {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "initialize");
    QDataStream stream(m_pluginCommunication);
    stream << data << '\n';
}

void PluginCommunication::clear() {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "clear");
    QDataStream stream(m_pluginCommunication);
    stream << data << '\n';
}

void PluginCommunication::configChanged(const QByteArray& configid, const QVariantMap& data) {
    if (!m_pluginCommunication)
        return;
    QVariantMap mdata(data);
    ServiceData::setMethod(mdata, "configChanged");
    ServiceData::setConfigurationkey(mdata, configid);
    QDataStream stream(m_pluginCommunication);
    stream << mdata << '\n';
}

void PluginCommunication::requestProperties(int sessionid) {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "requestProperties");
    ServiceData::setSessionID(data, sessionid);
    QDataStream stream(m_pluginCommunication);
    stream << data << '\n';
}

void PluginCommunication::unregister_event(const QString& eventid) {
    if (!m_pluginCommunication)
        return;
    QVariantMap mdata;
    ServiceData::setMethod(mdata, "unregister_event");
    mdata[QLatin1String("eventid")] = eventid;
    QDataStream stream(m_pluginCommunication);
    stream << mdata << '\n';
}

void PluginCommunication::session_change(int sessionid, bool running)
{
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "session_change");
    ServiceData::setSessionID(data, sessionid);
    data[QLatin1String("running")] = running;
    QDataStream stream(m_pluginCommunication);
    stream << data << '\n';
}

bool PluginCommunication::callQtSlot(const QVariantMap& methodAndArguments, QVariant* returnValue) {
    if (!methodAndArguments.contains(QLatin1String("method_"))) return false;
    // Block signals to not call readyRead by the event system for the next read
    m_pluginCommunication->blockSignals(true);
    QVariantMap mdata;
    QDataStream stream(m_pluginCommunication);
    stream << methodAndArguments << '\n';
    m_pluginCommunication->waitForBytesWritten();
    if (returnValue) {
        m_pluginCommunication->waitForReadyRead(3000);
        if (m_pluginCommunication->canReadLine()) {
            stream >> *returnValue;
        }
    }
    m_pluginCommunication->blockSignals(false);
    // signals were blocked. Check if new data arrived in the meantime
    readyRead();
    return true;
}

void PluginCommunication::disconnected() {
    if (id.size())
        m_controller->removePlugin(id);
    else
        m_controller->removePluginFromPending(this);
}

void PluginCommunication::startTimeout() {
    if (!id.size()) {
        qWarning() << "Server: Plugin process did not send pluginid";
        disconnected();
    }
}












PluginProcess::PluginProcess(PluginController* controller, const QString& filename)
        : m_controller(controller), m_filename(filename), m_aboutToFree(false) {
    m_pluginProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    connect(&m_pluginProcess, SIGNAL(finished(int)), SLOT(finished(int)));
    m_pluginProcess.start(filename);
    if (m_pluginProcess.waitForStarted())
        qDebug() << "Started plugin process" << filename;
    else
        qWarning() << "Failed starting plugin process" << filename;
    // Start timer for killing process if it does not establish a communication line
    //QTimer::singleShot(3000, this, SLOT(startTimeout()));
}

PluginProcess::~PluginProcess() {
    m_aboutToFree = true;
    if (m_pluginProcess.state()==QProcess::Running) {
        qDebug() << "Server: Terminate plugin" << m_filename << m_pluginProcess.pid();
        m_pluginProcess.terminate();
        m_pluginProcess.waitForFinished();
    }
}

void PluginProcess::finished(int) {
  if (m_pluginProcess.exitStatus()==QProcess::CrashExit)
    qWarning() << "Server: Plugin crashed" << m_filename << m_pluginProcess.pid();
  else
    qDebug() << "Server: Plugin finished" << m_filename << m_pluginProcess.pid();
    if (!m_aboutToFree)
        m_controller->removeProcess(this);
}

void PluginProcess::startTimeout() {
    qWarning() << "Server: Plugin process timeout" << m_filename << m_pluginProcess.pid();
    if (m_pluginProcess.state()==QProcess::Running) {
        m_pluginProcess.kill();
        m_pluginProcess.waitForFinished();
    }
    m_controller->removeProcess(this);
}
QLocalSocket* PluginCommunication::getSocket() {
    return m_pluginCommunication;
}
