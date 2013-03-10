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
#include "../utils/logging.h"
#include "../utils/paths.h"

/// The name of the server communication socket
#define LOCALSOCKETNAMESPACE "sceneserver"

static void catch_int(int )
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    if (!QProcessEnvironment().contains(QLatin1String("CHILDOFSERVER=1")))
        QCoreApplication::exit(0);
}

AbstractPlugin* AbstractPlugin::createInstance(const QByteArray& pluginid, const QByteArray& instanceid, const QByteArray& serverip, const QByteArray& port)
{
	setLogOptions(pluginid+":"+instanceid, true);
	qInstallMsgHandler(roomMessageOutput);
	
	AbstractPlugin* plugin = new AbstractPlugin(pluginid, instanceid);
	if (!plugin->createCommunicationSockets(serverip, port.toInt())) {
		delete plugin;
		return 0;
	}
	
	//set up signal handlers to exit on 2x CTRL+C
	signal(SIGINT, catch_int);
	signal(SIGTERM, catch_int);
	
	return plugin;
}

AbstractPlugin::AbstractPlugin(const QString& pluginid, const QString& instanceid) : m_lastsessionid(-1), m_pluginid(pluginid), m_instanceid(instanceid)
{
}

AbstractPlugin::~AbstractPlugin() {}


QSslKey AbstractPlugin::readKey(const QString& fileKeyString)
{
	QFile fileKey(fileKeyString);
	if (fileKey.open(QIODevice::ReadOnly))
	{
		QByteArray key = fileKey.readAll();
		fileKey.close();
		return QSslKey(key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "1234");
	}
	else
	{
		qWarning() << fileKey.errorString();
		return QSslKey();
	}
}

QSslCertificate AbstractPlugin::readCertificate(const QString& filename)
{
	QFile fileCert(filename);
	if (fileCert.open(QIODevice::ReadOnly))
	{
		QByteArray cert = fileCert.readAll();
		fileCert.close();
		return QSslCertificate(cert);
	}
	else
	{
		qWarning() << fileCert.errorString();
		return QSslCertificate();
	}
}

void AbstractPlugin::sslErrors ( const QList<QSslError> & errors ) {
	QList<QSslError> filteredErrors(errors);
	for (int i=filteredErrors.size()-1;i>=0;--i)
		if (filteredErrors[i].error() == QSslError::SelfSignedCertificate || filteredErrors[i].error() == QSslError::HostNameMismatch)
			filteredErrors.removeAt(i);
		if (filteredErrors.size())
			qWarning() << "SSL Errors" << filteredErrors;
}

bool AbstractPlugin::createCommunicationSockets(const QByteArray& serverip, int port)
{
	// ssl cert and keys
	ignoreSslErrors();
	setProtocol(QSsl::SslV3);
	setPeerVerifyMode(QSslSocket::VerifyNone);
	
	// Add client key
	{
		QByteArray filename;
		filename.append("services/").append(m_pluginid.toLatin1()).append(".key");
		QSslKey sslKeySpecific = readKey(setup::certificateFile(filename.constData()));
		if (sslKeySpecific.isNull()) {
			qDebug() << "SSL specific key invalid:" << setup::certificateFile(filename.constData())<<"Will try generic one.";
			QSslKey sslKeyGeneric = readKey(setup::certificateFile("services/generic.key"));
			if (sslKeyGeneric.isNull()) {
				qWarning() << "SSL key invalid:" << setup::certificateFile("services/generic.key");
				return false;
			} else
				setPrivateKey(sslKeyGeneric);
		} else
			setPrivateKey(sslKeySpecific);
	}
	
	// Set public certificate
	{
		QByteArray filename;
		filename.append("services/").append(m_pluginid.toLatin1()).append(".crt");
		QSslCertificate sslCertSpecific = readCertificate(setup::certificateFile(filename.constData()));
		if (sslCertSpecific.isNull()) {
			qDebug() << "SSL specific Certificate invalid:" << setup::certificateFile(filename.constData())<<"Will try generic one.";
			QSslCertificate sslCertGeneric = readCertificate(setup::certificateFile("services/generic.crt"));
			if (sslCertGeneric.isNull()) {
				qWarning() << "SSL Certificate invalid:" << setup::certificateFile("services/generic.crt");
				return false;
			} else
				setLocalCertificate(sslCertGeneric);
		} else
			setLocalCertificate(sslCertSpecific);
	}

	// Add public certificate of the server to the trusted hosts
	QSslCertificate sslCertServer = readCertificate(setup::certificateFile("server.crt"));
	if (sslCertServer.isNull())
	{
		qWarning() << "SSL Server Certificate invalid:" << setup::certificateFile("server.crt");
		return false;
	}
	addCaCertificate(sslCertServer);

	// connect
	connectToHostEncrypted(serverip, port);
	if (!waitForConnected()) {
		return false;
	}
	// The core will send a version identifier
	if (!waitForReadyRead()) {
		return false;
	}
	{
		// we expect the server (remote_types=="core") and at least api level 10
		SceneDocument serverident(readLine());
		if (!serverident.isMethod("identify") ||
			serverident.toInt("apiversion")<10 ||
			serverident.toString("remote_types")!=QLatin1String("core")) {
			qWarning()<<"Wrong api version!";
			return false;
		}
		SceneDocument ack;
		ack.setComponentID(m_pluginid);
		ack.setInstanceID(m_instanceid);
		ack.makeack(serverident.requestid());
		write(ack.getjson());
		flush();
	}
	
	// identify
	SceneDocument identify;
	identify.setrequestid();
	identify.setMethod("identify");
	identify.setComponentID(m_pluginid);
	identify.setInstanceID(m_instanceid);
	identify.setData("remote_types",QStringList() << QLatin1String("service"));
	identify.setData("apiversion", 10);
	write(identify.getjson());
	flush();
	if (!waitForReadyRead()) {
		return false;
	}
	SceneDocument serverresponse(readLine());
	if (!serverresponse.isResponse(identify)) {
		qWarning()<<"Couldn't identify to core!";
		return false;
	}
	
    connect(this, SIGNAL(disconnected()), SLOT(disconnectedFromServer()));
	connect(this, SIGNAL(readyRead()), SLOT(readyReadCommunication()));
    return true;
}

void AbstractPlugin::readyReadCommunication()
{
    while (canReadLine()) {
		SceneDocument doc(readLine());

        m_lastsessionid = doc.sessionid();

        // Retrieve method
        const QByteArray method = doc.method();

        // Server callable methods
        if (method == "configChanged") {
            configChanged(doc.id().toUtf8(), doc.getData());
        } else if (method == "methodresponse") { // Ignore responses
        } else if (method == "initialize") {
            initialize();
        } else if (method == "clear") {
            clear();
        } else if (method == "requestProperties") {
            requestProperties(m_lastsessionid);
        } else if (method == "session_change") {
            session_change(m_lastsessionid, doc.toBool("running"));
		} else if (method == "call") { // server and other plugins callable methods
			// Extract data document out of the transfered doc
			SceneDocument dataDocument(doc.getData().value(QLatin1String("doc")).toMap());
            // Prepare response
			SceneDocument transferdoc;
			transferdoc.makeack(doc.requestid());
			transferdoc.setComponentID(m_pluginid);
			transferdoc.setInstanceID(m_instanceid);
			
			int methodId = invokeHelperGetMethodId(dataDocument.method());
            // If method not found call dataFromPlugin
            if (methodId == -1) {
				qWarning() << "Method not found!" << dataDocument.method();
				transferdoc.setData("error", "Method not found!");
				write(transferdoc.getjson());
                continue;
            }
			
            QVector<QVariant> argumentsInOrder(9);
            int params;
			if ((params = invokeHelperMakeArgumentList(methodId, dataDocument.getData(), argumentsInOrder)) == -1) {
				qWarning() << "Arguments list incompatible!" << dataDocument.method() << methodId << dataDocument.getData() << argumentsInOrder;
                transferdoc.setData("error", "Arguments list incompatible!");
				write(transferdoc.getjson());
                continue;
            }

            const char* returntype = metaObject()->method(methodId).typeName();
            // If no response is expected, write the method-response message before invoking the target method,
            // because that may write data the the server and the response-message have to be the first answer
			QByteArray responseid = doc.requestid().toLatin1();
            transferdoc.setData("responseid_", responseid);
            transferdoc.setData("response_",
								invokeSlot(dataDocument.method(), params, returntype,
											argumentsInOrder[0], argumentsInOrder[1], argumentsInOrder[2], argumentsInOrder[3],
											argumentsInOrder[4], argumentsInOrder[5], argumentsInOrder[6], argumentsInOrder[7], argumentsInOrder[8]));
            if (responseid.size()) {
				write(transferdoc.getjson());
                qDebug() << "WRITE RESPONSE" << transferdoc.toString("response_");
            }
        }
    }
}

bool AbstractPlugin::callRemoteComponent( const QVariantMap& dataout )
{
	SceneDocument d;
	d.setComponentID(m_pluginid);
	d.setInstanceID(m_instanceid);
	d.setData("doc", dataout);
	d.setMethod("call");
	write(d.getjson());
	flush();
	return true;
}

void AbstractPlugin::changeConfig(const QByteArray& configid, const QVariantMap& data) {
	SceneDocument transferdoc(data);
	transferdoc.setMethod("changeConfig");
	transferdoc.setComponentID(m_pluginid);
	transferdoc.setInstanceID(m_instanceid);
	transferdoc.setid(configid);
	write(transferdoc.getjson());
}

void AbstractPlugin::changeProperty(const QVariantMap& data, int sessionid) {
	SceneDocument transferdoc(data);
	transferdoc.setMethod("changeProperty");
	transferdoc.setComponentID(m_pluginid);
	transferdoc.setInstanceID(m_instanceid);
	transferdoc.setSessionID(sessionid);
	write(transferdoc.getjson());
}

void AbstractPlugin::eventTriggered(const QString& eventid, const QString& dest_sceneid) {
	SceneDocument transferdoc;
	transferdoc.setMethod("eventTriggered");
	transferdoc.setComponentID(m_pluginid);
	transferdoc.setInstanceID(m_instanceid);
	transferdoc.setSceneid(dest_sceneid);
	transferdoc.setid(eventid);
	write(transferdoc.getjson());
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
        } else { // Otherwise create a QVariant and copy that to the output QVector
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
		qWarning() << __FUNCTION__ << "failed:"<<methodname;
    }
    return result;
}
QString AbstractPlugin::pluginid() {
    return m_pluginid;
}
QString AbstractPlugin::instanceid() {
    return m_instanceid;
}
int AbstractPlugin::getLastSessionID() {return m_lastsessionid;}
