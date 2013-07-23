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
#include <QTimer>
#include "logging.h"
#include "paths.h"

static void catch_int(int )
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    if (!QProcessEnvironment().contains(QLatin1String("CHILDOFSERVER=1")))
        QCoreApplication::exit(0);
}

AbstractPlugin::AbstractPlugin() : m_lastsessionid(-1){}

void AbstractPlugin::setPluginInfo(const QByteArray& pluginid, const QByteArray& instanceid)
{
	setLogOptions(pluginid+":"+instanceid, true);
	m_pluginid = pluginid;
	m_instanceid = instanceid;
	qInstallMsgHandler(roomMessageOutput);
	
	//set up signal handlers to exit on 2x CTRL+C
	signal(SIGINT, catch_int);
	signal(SIGTERM, catch_int);
}

AbstractPlugin::~AbstractPlugin() {}


QSslKey AbstractPlugin::readKey(const QString& fileKeyString, bool ignoreNotExisting)
{
	QFile fileKey(fileKeyString);
	if (!fileKey.exists()) {
		if (!ignoreNotExisting) qWarning() << "File does not exist:"<< fileKeyString;
		return QSslKey();
	}
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

QSslCertificate AbstractPlugin::readCertificate(const QString& filename, bool ignoreNotExisting)
{
	QFile fileCert(filename);
	if (!fileCert.exists()) {
		if (!ignoreNotExisting) qWarning() << "File does not exist:"<< filename;
		return QSslCertificate();
	}
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
	socket.ignoreSslErrors();
	socket.setProtocol(QSsl::SslV3);
	socket.setPeerVerifyMode(QSslSocket::VerifyNone);
	bool useGenericKey = false, useGenericCert = false;
	
	// Add client key
	{
		QByteArray filename;
		filename.append("services/").append(m_pluginid.toLatin1()).append(".key");
		QSslKey sslKeySpecific = readKey(setup::certificateFile(filename.constData()), true);
		if (sslKeySpecific.isNull()) {
			QSslKey sslKeyGeneric = readKey(setup::certificateFile("services/generic.key"), false);
			if (sslKeyGeneric.isNull()) {
				qWarning() << "SSL key invalid:" << setup::certificateFile("services/generic.key");
				return false;
			} else {
				useGenericKey = true;
				socket.setPrivateKey(sslKeyGeneric);
			}
		} else
			socket.setPrivateKey(sslKeySpecific);
	}
	
	// Set public certificate
	{
		QByteArray filename;
		filename.append("services/").append(m_pluginid.toLatin1()).append(".crt");
		QSslCertificate sslCertSpecific = readCertificate(setup::certificateFile(filename.constData()), true);
		if (sslCertSpecific.isNull()) {
			QSslCertificate sslCertGeneric = readCertificate(setup::certificateFile("services/generic.crt"), false);
			if (sslCertGeneric.isNull()) {
				qWarning() << "SSL Certificate invalid:" << setup::certificateFile("services/generic.crt");
				return false;
			} else {
				useGenericCert = true;
				socket.setLocalCertificate(sslCertGeneric);
			}
		} else
			socket.setLocalCertificate(sslCertSpecific);
	}

	// Add public certificate of the server to the trusted hosts
	QSslCertificate sslCertServer = readCertificate(setup::certificateFile("server.crt"), true);
	if (sslCertServer.isNull())
	{
		qWarning() << "SSL Server Certificate invalid:" << setup::certificateFile("server.crt");
		return false;
	}
	socket.addCaCertificate(sslCertServer);

	// connect
	socket.connectToHostEncrypted(serverip, port);
	if (!socket.waitForConnected()) {
		qWarning() << "Server does not respond!"<<serverip<< port;
		return false;
	}
	// The core will send a version identifier. If it doesn't in time
	// we exit here. If the socket is no longer connected at this stage,
	// the core closed the connection (wrong certificate for example).
	if (!socket.waitForReadyRead()) {
		// We do not print a message if no longer connected. The core
		// will probably output the problem.
		if (socket.state() == QAbstractSocket::ConnectedState)
			qWarning() << "Server didn't send an identify message!";
		return false;
	}
	{
		// we expect the server (remote_types=="core") and at least api level 10
		SceneDocument serverident(socket.readLine());
		if (!serverident.isMethod("identify") || serverident.toInt("apiversion")<10 ||
			serverident.toString("provides")!=QLatin1String("core")) {
			qWarning()<<"Wrong api version!";
			return false;
		}
		SceneDocument ack;
		ack.makeack(serverident.requestid());
		socket.write(ack.getjson());
		socket.flush();
	}
	
	// identify
	SceneDocument identify;
	identify.setrequestid();
	identify.setMethod("identify");
	identify.setComponentID(m_pluginid);
	identify.setInstanceID(m_instanceid);
	identify.setData("provides",QStringList() << QLatin1String("service"));
	identify.setData("useGenericKey",useGenericKey);
	identify.setData("useGenericCert",useGenericCert);
	identify.setData("apiversion", 10);
	socket.write(identify.getjson());
	socket.flush();
	if (!socket.waitForReadyRead()) {
		qWarning() << "Server does not send an ack!";
		return false;
	}
	SceneDocument serverresponse(socket.readLine());
	if (!serverresponse.isResponse(identify)) {
		qWarning()<<"Couldn't identify to core!";
		return false;
	}
	
    connect(&socket, SIGNAL(disconnected()), SLOT(disconnectedFromServer()));
	connect(&socket, SIGNAL(readyRead()), SLOT(readyReadCommunication()));
	
	// work through rest of received data with the next event loop step
	if (socket.canReadLine())
		QTimer::singleShot(0, this, SLOT(readyReadCommunication()));
    return true;
}

void AbstractPlugin::readyReadCommunication()
{
    while (socket.canReadLine()) {
		SceneDocument transferdoc;
		if (executeMethodByIncomingDocument(SceneDocument(socket.readLine()), transferdoc))
			socket.write(transferdoc.getjson());
    }
}

bool AbstractPlugin::callRemoteComponent( const QVariantMap& dataout )
{
	SceneDocument d;
	d.setComponentID(m_pluginid);
	d.setInstanceID(m_instanceid);
	d.setData("doc", dataout);
	d.setMethod("call");
	socket.write(d.getjson());
	socket.flush();
	return true;
}

void AbstractPlugin::changeConfig(const QByteArray& configid, const QVariantMap& data) {
	SceneDocument transferdoc(data);
	transferdoc.setMethod("changeConfig");
	transferdoc.setid(configid);
	socket.write(transferdoc.getjson());
}

void AbstractPlugin::changeProperty(const QVariantMap& data, int sessionid) {
	SceneDocument transferdoc(data);
	transferdoc.setSessionID(sessionid);
	socket.write(transferdoc.getjson());
}

void AbstractPlugin::eventTriggered(const QString& eventid, const QString& dest_sceneid) {
	SceneDocument transferdoc;
	transferdoc.setMethod("eventTriggered");
	transferdoc.setSceneid(dest_sceneid);
	transferdoc.setid(eventid);
	socket.write(transferdoc.getjson());
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
            return 10;
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

bool AbstractPlugin::addEvent(const QVariantMap& data) {
	SceneDocument ignore;
	return executeMethodByIncomingDocument(data, ignore);
}

bool AbstractPlugin::removeEvent(const QVariantMap& data) {
	SceneDocument ignore;
	return executeMethodByIncomingDocument(data, ignore);
}

bool AbstractPlugin::executeMethodByIncomingDocument(const SceneDocument& dataDocument, SceneDocument& responseDocument) {
	if (dataDocument.isType(SceneDocument::TypeAck))
		return false;
	
	// 		qDebug() << "receive" << dataDocument.getjson();
	m_lastsessionid = dataDocument.sessionid();
	
	// Prepare response
	responseDocument.makeack(dataDocument.requestid());
	
	int methodId = invokeHelperGetMethodId(dataDocument.method());
	// If method not found call dataFromPlugin
	if (methodId == -1) {
		qWarning() << "Method not found!" << dataDocument.method();
		responseDocument.setData("error", "Method not found!");
		socket.write(responseDocument.getjson());
		return false;
	}
	
	QVector<QVariant> argumentsInOrder(9);
	int params;
	if ((params = invokeHelperMakeArgumentList(methodId, dataDocument.getData(), argumentsInOrder)) == -1) {
		qWarning() << "Arguments list incompatible!" << dataDocument.method() << methodId << dataDocument.getData() << argumentsInOrder;
		responseDocument.setData("error", "Arguments list incompatible!");
		socket.write(responseDocument.getjson());
		return false;
	}
	
	const char* returntype = metaObject()->method(methodId).typeName();
	// If no response is expected, write the method-response message before invoking the target method,
	// because that may write data the the server and the response-message have to be the first answer
	responseDocument.setData("response_",
						invokeSlot(dataDocument.method(), params, returntype,
								   argumentsInOrder[0], argumentsInOrder[1], argumentsInOrder[2], argumentsInOrder[3],
				 argumentsInOrder[4], argumentsInOrder[5], argumentsInOrder[6], argumentsInOrder[7], argumentsInOrder[8]));
	return true;
}
