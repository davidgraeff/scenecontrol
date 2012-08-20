#include "abstractplugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMetaMethod>
#include <QBitArray>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <QElapsedTimer>
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <QProcessEnvironment>

#define MAGICSTRING "roomcontrol_"

static QByteArray pluginidstatic;
static void catch_int(int )
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    if (!QProcessEnvironment().contains(QLatin1String("CHILDOFSERVER=1")))
        QCoreApplication::exit(0);
}

void roomMessageOutput(QtMsgType type, const char *msg)
{
    time_t rawtime;
    tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );

    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "[%2d:%02d,%s] %s\n", (ptm->tm_hour+1)%24, ptm->tm_min, pluginidstatic.constData(), msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "[%2d:%02d,%s] \033[33mWarning: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, pluginidstatic.constData(), msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[%2d:%02d,%s] \033[31mCritical: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, pluginidstatic.constData(), msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[%2d:%02d,%s] \033[31mFatal: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, pluginidstatic.constData(), msg);
        abort();
    }
}

AbstractPlugin::AbstractPlugin(const QString& pluginid, const QString& instanceid) : m_lastsessionid(-1), m_pluginid(pluginid), m_instanceid(pluginid)
{
	pluginidstatic = m_pluginid.toUtf8();
    qInstallMsgHandler(roomMessageOutput);
    connect(this, SIGNAL(newConnection()), SLOT(newConnectionCommunication()));

    //set up signal handlers to exit on 2x CTRL+C
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
}

AbstractPlugin::~AbstractPlugin()
{
	close();
	qDeleteAll(m_connectionsByID);
	m_connectionsByID.clear();
}

bool AbstractPlugin::createCommunicationSockets()
{
    // create server socket for incoming connections
    const QString name = QLatin1String(MAGICSTRING) + m_pluginid;
    removeServer(name);
    if (!listen(name)) {
        qWarning() << "Plugin interconnect server failed";
        return false;
    }
    // create connection to server
    QLocalSocket* socketToServer = getClientConnection(COMSERVERSTRING);
    if (!socketToServer) {
        qWarning() << "Couldn't connect to server";
        return false;
    }
    connect(socketToServer, SIGNAL(disconnected()), SLOT(disconnectedFromServer()));
    return true;
}

void AbstractPlugin::readyReadCommunication()
{
    QLocalSocket* socket = (QLocalSocket*)sender();

    const QByteArray& plugin_id = m_connectionsBySocket.value(socket);
    if (plugin_id.isEmpty()) {
        qWarning() << "Receiving failed";
        socket->deleteLater();
        return;
    }

    m_chunk.append(socket->readAll());

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
		
		SceneDocument doc(variantdata);

        m_lastsessionid = doc.sessionid();

        // Retrieve method
        const QByteArray method = doc.method();

//         if (method.size())
//             qDebug() << plugin_id << "calls method" << method;
//         else
//             qDebug() << "Received from" << plugin_id << variantdata;

		// The received data do not contain a method: This have to be payload from another plugin
        if (!method.size()) {
            dataFromPlugin(plugin_id, variantdata);
            continue;
        }
        // Predefined methods
        if (method == "pluginid") {
			SceneDocument transferdoc;
			transferdoc.setMethod("pluginid");
			transferdoc.setPluginid(m_pluginid);
            writeToSocket(socket, transferdoc.getData());
        } else if (method == "configChanged") {
            const QByteArray key = doc.configurationkey();
            configChanged(key, variantdata);
        } else if (method == "methodresponse") { // Ignore responses
        } else if (method == "initialize") {
            initialize();
        } else if (method == "clear") {
            clear();
        } else if (method == "requestProperties") {
            requestProperties(m_lastsessionid);
        } else if (method == "session_change") {
            session_change(m_lastsessionid, doc.toBool("running"));
        } else if (method == "unregister_event") {
            unregister_event(doc.toString("eventid"));
        } else {
            // Prepare response
			SceneDocument transferdoc;
			transferdoc.setMethod("methodresponse");
			transferdoc.setPluginid(m_pluginid);
			
            int methodId = invokeHelperGetMethodId(method);
            // If method not found call dataFromPlugin
            if (methodId == -1) {
                qWarning() << "Method not found!" << method;
				transferdoc.setData("error", "Method not found!");
                writeToSocket(socket, transferdoc.getData());
                continue;
            }

            QVector<QVariant> argumentsInOrder(9);
            int params;
            if ((params = invokeHelperMakeArgumentList(methodId, variantdata, argumentsInOrder)) == -1) {
				qWarning() << "Arguments list incompatible!" << method << methodId << variantdata << argumentsInOrder;
                transferdoc.setData("error", "Arguments list incompatible!");
                writeToSocket(socket, transferdoc.getData());
                continue;
            }

            const char* returntype = metaObject()->method(methodId).typeName();
            // If no response is expected, write the method-response message before invoking the target method,
            // because that may write data the the server and the response-message have to be the first answer
            QByteArray responseid = variantdata.value(QLatin1String("responseid_")).toByteArray();
            transferdoc.setData("responseid_", responseid);
            transferdoc.setData("response_",
								invokeSlot(method, params, returntype,
											argumentsInOrder[0], argumentsInOrder[1], argumentsInOrder[2], argumentsInOrder[3],
											argumentsInOrder[4], argumentsInOrder[5], argumentsInOrder[6], argumentsInOrder[7], argumentsInOrder[8]));
            if (responseid.size()) {
                writeToSocket(socket, transferdoc.getData());
                qDebug() << "WRITE RESPONSE" << transferdoc.toString("response_");
            }
        }
    }
}


void AbstractPlugin::writeToSocket(QLocalSocket* socket, const QVariantMap& data) {
    QDataStream streamout(socket);
    streamout << data;
    streamout.writeRawData("\n\t\0", 3);
}

void AbstractPlugin::newConnectionCommunication()
{
    while (hasPendingConnections()) {
        QLocalSocket * socket = nextPendingConnection ();
        QElapsedTimer t;
        t.start();
        while (!t.hasExpired(2000) && !socket->canReadLine()) {
            QCoreApplication::processEvents();
        }

        if (socket->canReadLine()) {
            const QByteArray plugin_id = socket->readLine();
            m_connectionsByID[plugin_id] = socket;
            m_connectionsBySocket[socket] = plugin_id;
            connect(socket, SIGNAL(readyRead()), SLOT(readyReadCommunication()));
        } else {
            qWarning()<<"Socket tried to connect: No authentification" << socket->bytesAvailable();
            socket->deleteLater();
        }
    }
}

bool AbstractPlugin::sendDataToPlugin(const QByteArray& plugin_id, const QVariantMap& dataout)
{
    QLocalSocket* socket = getClientConnection(plugin_id);
    if (!socket) {
        qWarning() << "Failed to send data to" << plugin_id;
        return false;
    }
    writeToSocket(socket, dataout);
    return true;
}

QLocalSocket* AbstractPlugin::getClientConnection(const QByteArray& plugin_id) {
    // If this connection is known we get a valid socket out of the map (id->socket)
    QLocalSocket* socket = m_connectionsByID.value(plugin_id);
    // Try to connect to the target plugin if no connection is made so far
    if (!socket) {
        socket = new QLocalSocket();
        socket->connectToServer(QLatin1String(MAGICSTRING) + plugin_id);
        connect(socket, SIGNAL(readyRead()), SLOT(readyReadCommunication()));
        // wait for at least 30 seconds for a connection
        if (!socket->waitForConnected()) {
            delete socket;
            return 0;
        }
        // connection established: add to map, send welcome string with current plugin id
        m_connectionsByID[plugin_id] = socket;
        m_connectionsBySocket[socket] = plugin_id;
		SceneDocument transferdoc;
		transferdoc.setMethod("identifyplugin");
		transferdoc.setPluginid(m_pluginid);
		transferdoc.setPlugininstance(m_instanceid);
		writeToSocket(socket, transferdoc.getData());
        socket->waitForBytesWritten();
    }
    return socket;
}

void AbstractPlugin::changeConfig(const QByteArray& category, const QVariantMap& data) {
	SceneDocument transferdoc(data);
	transferdoc.setMethod("changeConfig");
	transferdoc.setPluginid(m_pluginid);
	transferdoc.setPlugininstance(m_instanceid);
	transferdoc.setConfigurationkey(category);
	sendDataToPlugin(COMSERVERSTRING, transferdoc.getData());
}

void AbstractPlugin::changeProperty(const QVariantMap& data, int sessionid) {
	SceneDocument transferdoc(data);
	transferdoc.setMethod("changeProperty");
	transferdoc.setPluginid(m_pluginid);
	transferdoc.setPlugininstance(m_instanceid);
	transferdoc.setSessionID(sessionid);
	sendDataToPlugin(COMSERVERSTRING, transferdoc.getData());
}

void AbstractPlugin::eventTriggered(const QString& eventid, const QString& dest_sceneid) {
	SceneDocument transferdoc;
	transferdoc.setMethod("eventTriggered");
	transferdoc.setPluginid(m_pluginid);
	transferdoc.setPlugininstance(m_instanceid);
	transferdoc.setSceneid(dest_sceneid);
	transferdoc.setid(eventid);
	sendDataToPlugin(COMSERVERSTRING, transferdoc.getData());
}

void AbstractPlugin::disconnectedFromServer() {
    QCoreApplication::exit(0);
}

int AbstractPlugin::invokeHelperGetMethodId(const QByteArray& methodName) {
    const int c = metaObject()->methodCount();
    for (int i=0;i<c;++i) {
        // Extract method name from signature of QMetaMethod
        QByteArray methodNameOfPlugin(metaObject()->method(i).signature());
        methodNameOfPlugin.resize(methodNameOfPlugin.indexOf('('));

        if (methodNameOfPlugin == methodName)
            return i;
    }
    return -1;
}

int AbstractPlugin::invokeHelperMakeArgumentList(int methodID, const QVariantMap& inputData, QVector< QVariant >& output) {
    QList<QByteArray> parameterNames = metaObject()->method(methodID).parameterNames();
    QList<QByteArray> parameterTypes = metaObject()->method(methodID).parameterTypes();
    const int numParams = parameterNames.size();
    for (int paramNameIndex=0;paramNameIndex<numParams && paramNameIndex<output.size();++paramNameIndex) {
        // Look for a key in inputData that matches the current parameter name
        const QVariantMap::const_iterator paramPosition = inputData.find(QString::fromAscii(parameterNames[paramNameIndex]));
        // If not found abort
        if (paramPosition == inputData.end()) {
            qWarning() << "Method parameter missing!" << parameterNames[paramNameIndex] << inputData;
            return -1;
        } else { // Otherwise create a QVariant a copy that to the output QVector
            output[paramNameIndex] = paramPosition.value();
        }
    }
    return numParams;
}

#define QX_ARG(i) ((numParams>i)?QGenericArgument(p ## i .typeName(), p ## i .constData()):QGenericArgument())
QVariant AbstractPlugin::invokeSlot(const QByteArray& methodname, int numParams, const char* returntype, QVariant p0, QVariant p1, QVariant p2, QVariant p3, QVariant p4, QVariant p5, QVariant p6, QVariant p7, QVariant p8) {
    Q_UNUSED(returntype);
    QVariant result(QVariant::nameToType(returntype));
    bool ok = QMetaObject::invokeMethod(this, methodname, QGenericReturnArgument(returntype,result.data()), QX_ARG(0), QX_ARG(1), QX_ARG(2), QX_ARG(3), QX_ARG(4), QX_ARG(5), QX_ARG(6), QX_ARG(7), QX_ARG(8));
    if (!ok) {
        qWarning() << __FUNCTION__ << "failed";
    }
    return result;
}
QString AbstractPlugin::pluginid() {
    return m_pluginid;
}
QString AbstractPlugin::instanceid() {
    return m_instanceid;
}