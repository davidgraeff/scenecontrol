#include "pluginprocess.h"
#include "plugincontroller.h"
#include "shared/pluginservicehelper.h"
#include "shared/database.h"
#include "socket.h"
#include "collectioncontroller.h"
#include <QElapsedTimer>
#include <QThread>
#include <QTimer>
#include <QDir>
#include "paths.h"
#include "config.h"

PluginCommunication::PluginCommunication(PluginController* controller, QLocalSocket* socket) : m_controller(controller) {
    m_pluginCommunication=socket;
    connect(m_pluginCommunication, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(m_pluginCommunication, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), SLOT(stateChanged(QLocalSocket::LocalSocketState)));
    // Request plugin id
    QVariantMap data;
    ServiceData::setMethod(data, "pluginid");
    writeToPlugin(data);
    // Waiting at most 7 seconds for a respond otherwise kill the connection
    QTimer::singleShot(7000, this, SLOT(startTimeout()));
    readyRead();
}

PluginCommunication::~PluginCommunication() {
    delete m_pluginCommunication;
}

void PluginCommunication::readyRead()
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
            initialize();

            QDir dir(setup::baseDir());
            if (!dir.cd(QLatin1String(ROOM_DATABASEIMPORTPATH)))
                qWarning() << "Server: Database initial import path not found!";
            else
                Database::instance()->verifyPluginData(id, dir.absolutePath());
            Database::instance()->requestPluginConfiguration(id);
            Database::instance()->requestEvents(id);
        } else if (method == "methodresponse") {
            emit qtSlotResponse(variantdata.value(QLatin1String("response_")),
                                variantdata.value(QLatin1String("responseid_")).toByteArray(), id);
        } else if (method == "changeConfig") {
            const QString key = ServiceData::configurationkey(variantdata);
            if (key.isEmpty()) {
                qWarning() << "Server: Request changeConfig for" << id <<"but no key provided";
                continue;
            }
            // store new configuration value in database
            Database::instance()->changePluginConfiguration(id, key, variantdata);
        } else if (method == "changeProperty") {
            // Get session id and remove id from QVariantMap
            const int sessionid = ServiceData::sessionid(variantdata);
            ServiceData::removeSessionID(variantdata);
            // propagate changed property
            Socket::instance()->propagateProperty(variantdata, sessionid);
        } else if (method == "eventTriggered") {
            const QString collectionid = ServiceData::collectionid(variantdata);
            if (collectionid.isEmpty()) {
                qWarning() << "Server: Request collection execution by event for" << id <<"but no collectionid provided";
                continue;
            }
            //qDebug() << "eventTriggered";
            CollectionController::instance()->requestExecutionByCollectionId(collectionid);
        } else {
            qWarning() << "Unknown data from plugin" << m_chunk;
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
    writeToPlugin(data);
}

void PluginCommunication::clear() {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "clear");
    writeToPlugin(data);
}

void PluginCommunication::configChanged(const QByteArray& configid, const QVariantMap& data) {
    if (!m_pluginCommunication)
        return;
    if (data.isEmpty()) {
        qWarning() << "configChanged data empty!";
        return;
    }
    QVariantMap mdata(data);
    ServiceData::setMethod(mdata, "configChanged");
    ServiceData::setConfigurationkey(mdata, configid);
    writeToPlugin(mdata);
}

void PluginCommunication::requestProperties(int sessionid) {
    if (!m_pluginCommunication)
        return;
    QVariantMap data;
    ServiceData::setMethod(data, "requestProperties");
    ServiceData::setSessionID(data, sessionid);
    writeToPlugin(data);
}

void PluginCommunication::unregister_event(const QString& eventid) {
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

void PluginCommunication::session_change(int sessionid, bool running)
{
    if (!m_pluginCommunication)
        return;
    QVariantMap mdata;
    ServiceData::setMethod(mdata, "session_change");
    ServiceData::setSessionID(mdata, sessionid);
    mdata[QLatin1String("running")] = running;
    writeToPlugin(mdata);
}

void PluginCommunication::callQtSlot(const QVariantMap& methodAndArguments, const QByteArray& responseid, int sessionid) {
    if (!ServiceData::hasMethod(methodAndArguments)) {
        qWarning() << "Call of qt slot without method" << methodAndArguments;
        return;
    }

    QVariantMap modified = methodAndArguments;
    modified[QLatin1String("responseid_")] = responseid;
    ServiceData::setSessionID(modified, sessionid);
    writeToPlugin(modified);
}

void PluginCommunication::stateChanged(QLocalSocket::LocalSocketState state) {
    if (state==QLocalSocket::ConnectedState)
        return;

    if (id.size())
        m_controller->unloadPlugin(id);
    else
        m_controller->removePluginFromPending(this);
}

void PluginCommunication::startTimeout() {
    if (!id.size()) {
        qWarning() << "Server: Plugin process did not send pluginid";
        m_pluginCommunication->disconnectFromServer();
    }
}

bool PluginCommunication::writeToPlugin(const QVariantMap& data) {
    // check state. Sometimes write calls are one after each other and the
    // event system does not get a chance to check the socket state in between
    if (m_pluginCommunication->state()!=QLocalSocket::ConnectedState) {
        stateChanged(m_pluginCommunication->state());
        return false;
    }
    // write data
    QDataStream streamout(m_pluginCommunication);
    streamout << data;
    streamout.writeRawData("\n\t\0", 3);
    return true;
}








PluginProcess::PluginProcess(PluginController* controller, const QString& filename)
        : m_controller(controller), m_filename(filename), m_aboutToFree(false) {
    m_pluginProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    connect(&m_pluginProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(finished(int,QProcess::ExitStatus)));
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

void PluginProcess::finished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus==QProcess::CrashExit && exitCode != 0)
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
