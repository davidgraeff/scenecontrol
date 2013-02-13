#include "socket.h"
#include "config.h"
#include <QDebug>
#include <qfile.h>
#include <qvarlengtharray.h>
#include <QSslKey>
#include "shared/jsondocuments/json.h"
#include "shared/utils/paths.h"
#include "plugins/plugincontroller.h"
#include "plugins/pluginprocess.h"
#include "scene/scenecontroller.h"
//openssl req -x509 -new -out server.crt -keyout server.key -days 365

#define __FUNCTION__ __FUNCTION__

ControlServerSocket::~ControlServerSocket()
{
}

ControlServerSocket::ControlServerSocket() : m_disabledSecureConnections(false) {
    if (listen(QHostAddress::Any, ROOM_LISTENPORT)) {
        qDebug() << "SSL TCPSocket Server ready on port" << ROOM_LISTENPORT;
    }
}

static ControlServerSocket* socketInstance = 0;
ControlServerSocket* ControlServerSocket::instance()
{
    if (!socketInstance)
        socketInstance = new ControlServerSocket();
    return socketInstance;
}

void ControlServerSocket::incomingConnection(int socketDescriptor)
{
    QSslSocket *socket = new QSslSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
		delete socket;
		return;
	}

	m_sockets.insert(socketDescriptor, socket);
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	connect(socket, SIGNAL(sslErrors (QList<QSslError>)), this, SLOT(sslErrors (QList<QSslError>)));
	socket->setProtocol(QSsl::SslV3);

	QList<QSslError> expectedSslErrors;
	
	// Set private key for SSL
	QFile fileKey(setup::certificateFile("server.key"));
	if (fileKey.open(QIODevice::ReadOnly))
	{
		QByteArray key = fileKey.readAll();
		fileKey.close();
		QSslKey sslKey(key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "1234");
		if (key.isNull()) {
			qWarning() << "key invalid";
		} else
			socket->setPrivateKey(sslKey);
	}
	else
	{
		qWarning() << fileKey.errorString();
	}

	// Set public certificate
	QFile fileCert(setup::certificateFile("server.crt"));
	if (fileCert.open(QIODevice::ReadOnly))
	{
		QByteArray cert = fileCert.readAll();
		fileCert.close();
		QSslCertificate sslCert(cert);
		if (sslCert.isNull()) {
			qWarning() << "sslCert invalid" << fileCert.fileName();
		} else {
			socket->setLocalCertificate(sslCert);
		}
	}
	else
	{
		qWarning() << fileCert.errorString();
	}
	
	// Add all other certificate files to the trusted ones.
	QStringList certfiles = setup::certificateClientFiles();
	while (certfiles.size()) {
		QFile fileCert(certfiles.takeLast());
		if (fileCert.open(QIODevice::ReadOnly))
		{
			QByteArray cert = fileCert.readAll();
			fileCert.close();
			QSslCertificate sslCert(cert);
			if (sslCert.isNull()) {
				qWarning() << "sslCert invalid" << fileCert.fileName();
			} else {
				socket->addCaCertificate(sslCert);
				// We allow self signed certificates
				QSslError error(QSslError::SelfSignedCertificate, sslCert);
				expectedSslErrors.append(error);
				QSslError error2(QSslError::HostNameMismatch, sslCert);
				expectedSslErrors.append(error2);
			}
		}
		else
		{
			qWarning() << fileCert.errorString();
		}
	}
	
	socket->ignoreSslErrors(expectedSslErrors);
	if (!m_disabledSecureConnections)
		socket->startServerEncryption();
	//qDebug() << "Add client ssl certificate" << fileCert.fileName();
	//qDebug() << "New connection" << socket->peerAddress();

	// Notify plugins of new session
	PluginController* pc = PluginController::instance();
	PluginController::iterator i = pc->begin();
	while (!i.eof()) {
		(*i)->session_change(socketDescriptor, true);
		++i;
	}
}

void ControlServerSocket::sslErrors ( const QList<QSslError> & errors ) {
	QSslSocket *socket = (QSslSocket*)sender();
	QList<QSslError> filteredErrors(errors);
	for (int i=filteredErrors.size()-1;i>=0;--i) {
		QSslError& e = filteredErrors[i];
		if (e.error() == QSslError::SelfSignedCertificate) {
			filteredErrors.removeAt(i);
			continue;
		}
		// Error
		qWarning() << "SSL Error:" << e.errorString();
		socket->disconnectFromHost();
	}
}

void ControlServerSocket::readyRead() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
    while (serverSocket->canReadLine()) {
		// Create a SceneDocument out of the raw data
        QByteArray rawdata = serverSocket->readLine();
		rawdata.chop(1);
        if (!rawdata.length())
            continue;
		
		// json parsing
		QVariant v;
		{
			JsonReader r;
			if (!r.parse(rawdata)) {
				SceneDocument responsedoc;
				responsedoc.setComponentID(QLatin1String("server"));
				responsedoc.setType(SceneDocument::TypeError);
				responsedoc.setid(QLatin1String("no_json"));
				responsedoc.setData("msg", "Failed to parse json" + r.errorString().toUtf8());
				serverSocket->write(responsedoc.getjson());
				qWarning()<<"Server Socket: Failed to parse json" << r.errorString() << rawdata;
				continue;
			}
			v = r.result();
		}
		
		// Analyse the scene document: We only accept TypeExecution
		SceneDocument doc(v.toMap());
		if ( !doc.isType ( SceneDocument::TypeExecution ) ) {
			SceneDocument responsedoc;
			responsedoc.setComponentID(QLatin1String("server"));
			responsedoc.setType(SceneDocument::TypeError);
			responsedoc.setid(QLatin1String("no_execution_type"));
			responsedoc.setData("msg", QLatin1String("No execution type"));
			serverSocket->write(responsedoc.getjson());
			continue;
		}
		
		const int sessionid = serverSocket->socketDescriptor();
		
		// Handle the case where the incoming scene document is dedicated for a plugin
		if ( doc.componentID() != QLatin1String ( "server" ))
		{
			if ( !PluginController::instance()->execute(doc, sessionid) )
			{
				SceneDocument responsedoc;
				responsedoc.setComponentID(QLatin1String("server"));
				responsedoc.setType(SceneDocument::TypeError);
				responsedoc.setid(QLatin1String("plugin_not_found"));
				responsedoc.setData("msg", QLatin1String("Plugin not found"));
				serverSocket->write(responsedoc.getjson());
				continue;
			}
			continue;
		}
		
		// Special case: a method of the server process should be executed. handle this as one of the if's.
		if ( doc.isMethod ( "requestAllProperties" ) )
		{
			PluginController::instance()->requestAllProperties ( sessionid );
			SceneDocument s = SceneDocument::createNotification("plugins");
			s.setData("plugins", PluginController::instance()->pluginids());
			s.setComponentID(QLatin1String("PluginController"));
			sendToClients(s.getjson(), sessionid);
		}
		
		else if ( doc.isMethod ( "requestProperty" ) && doc.getData().contains(QLatin1String("property")) )
		{
			SceneDocument property( doc.toMap("property") );
			qDebug() << "property requested" << property.getjson();
			PluginController::instance()->requestProperty ( property, sessionid );
		}
		
		else if ( doc.isMethod ( "removeDocument" ) )
		{
			DataStorage::instance()->removeDocument(SceneDocument(v.toMap().value(QLatin1String("doc")).toMap()));
		}
		
		else if ( doc.isMethod ( "changeDocument" ) )
		{
			DataStorage::instance()->storeDocument(SceneDocument(v.toMap().value(QLatin1String("doc")).toMap()), true);
		}
		
		else if ( doc.isMethod ( "createSceneItem" ) )
		{
			DataStorage::instance()->createSceneItem(SceneDocument(v.toMap().value(QLatin1String("scene")).toMap()).uid(),
													 SceneDocument(v.toMap().value(QLatin1String("sceneitem")).toMap()));
		}
		
		else if ( doc.isMethod ( "removeSceneItem" ) )
		{
			DataStorage::instance()->removeSceneItem(SceneDocument(v.toMap().value(QLatin1String("scene")).toMap()).uid(),
													 SceneDocument(v.toMap().value(QLatin1String("sceneitem")).toMap()).uid());
		}
		
		else if ( doc.isMethod ( "fetchAllDocuments" ) )
		{
			// Convert all SceneDocuments to QVariantMaps and store them in a QVariantList
			QVariantList l;
			QList<SceneDocument> docs;
			DataStorage::instance()->fetchAllDocuments(docs);
			for (int i=0;i<docs.size(); ++i) {
				l.append(docs[i].getData());
			}
			// Create the response
			SceneDocument s = SceneDocument::createNotification ( "alldocuments" );
			s.setComponentID ( QLatin1String ( "datastorage" ) );
			s.setData("documents", l);
			sendToClients ( s.getjson(), sessionid );
		}

		else if ( doc.isMethod ( "registerNotifier" ) )
		{
			// Register a StorageNotifier Object to the DataStorage
			DataStorage::instance()->registerNotifier(notifier(sessionid));
			
			// Create the response
			SceneDocument s = SceneDocument::createNotification ( "registerNotifier" );
			s.setComponentID ( QLatin1String ( "server" ) );
			s.setData("notifierstate", true); // registered
			sendToClients ( s.getjson(), sessionid );
		}
		
		else if ( doc.isMethod ( "unregisterNotifier" ) )
		{
			// Unregister a StorageNotifier Object from the DataStorage
			delete m_notifiers.take(sessionid);
			
			// Create the response
			SceneDocument s = SceneDocument::createNotification ( "registerNotifier" );
			s.setComponentID ( QLatin1String ( "server" ) );
			s.setData("notifierstate", false); // unregistered
			sendToClients ( s.getjson(), sessionid );
		}
		
		else if ( doc.isMethod ( "runcollection" ) )
		{
			SceneController::instance()->startScene ( doc.sceneid() );
		}

		else if ( doc.isMethod ( "version" ) )
		{
			SceneDocument s = SceneDocument::createNotification ( "version" );
			s.setData ( "version", QLatin1String ( ABOUT_VERSION ) );
			s.setComponentID ( QLatin1String ( "server" ) );
			sendToClients ( s.getjson(), sessionid );
		} else
		{
			SceneDocument responsedoc;
			responsedoc.setComponentID(QLatin1String("server"));
			responsedoc.setType(SceneDocument::TypeError);
			responsedoc.setid(QLatin1String("method_not_known"));
			responsedoc.setData("msg", QLatin1String("Method not known"));
			serverSocket->write(responsedoc.getjson());
		}
    }
}

void ControlServerSocket::socketDisconnected() {
    QSslSocket *socket = (QSslSocket *)sender();
    const int socketDescriptor = socket->socketDescriptor();
    m_sockets.remove(socketDescriptor);
    socket->deleteLater();

    // Notify plugins of finished session
    PluginController* pc = PluginController::instance();
	PluginController::iterator i = pc->begin();
	while (!i.eof()) {
		(*i)->session_change(socketDescriptor, true);
		++i;
	}

    // Remove DataStorage notifier
	if (m_notifiers.contains(socketDescriptor)) {
		delete m_notifiers.take(socketDescriptor);
	}
	
	if (socket->error() != QAbstractSocket::RemoteHostClosedError)
		qDebug() << "Unexpected socket close:" << socket->error();
}

void ControlServerSocket::sendToClients(const QByteArray& rawdata, int sessionid) {
    if (rawdata.isEmpty()) {
		qWarning() << "No data or no newline as last character:" << rawdata;
		return;
	}
	
	if (sessionid==-1) {
		QMap<int, QSslSocket*>::const_iterator it = m_sockets.constBegin();
		for (;it != m_sockets.constEnd();++it) {
			it.value()->write(rawdata);
			if (rawdata.right(1)!="\n")
				it.value()->write("\n");
		}
	} else {
		// try to find a socket connection with corresponding sessionid
		QSslSocket *socket = m_sockets.value(sessionid);
		if (socket) {
			socket->write(rawdata);
			if (rawdata.right(1)!="\n")
				socket->write("\n");
		} else {
			qWarning() << "SessionID not found:" << sessionid; 
		}
	}
}


StorageNotifierSocket* ControlServerSocket::notifier ( int sessionid ) {
	StorageNotifierSocket* s = m_notifiers[sessionid];
	if (!s) {
		s = new StorageNotifierSocket(sessionid);
		m_notifiers[sessionid] = s;
	}
	return s;
}

void StorageNotifierSocket::documentChanged(const QString& filename, SceneDocument* /*oldDoc*/, SceneDocument* newDoc) {
	// Create the response
	SceneDocument s = SceneDocument::createNotification ( "documentChanged" );
	s.setComponentID ( QLatin1String ( "datastorage" ) );
	s.setData("filename", filename);
	s.setData("document", newDoc->getData());
	ControlServerSocket::instance()->sendToClients ( s.getjson(), m_sessionid );
}

void StorageNotifierSocket::documentRemoved ( const QString& filename, SceneDocument* document ) {
	// Create the response
	SceneDocument s = SceneDocument::createNotification ( "documentRemoved" );
	s.setComponentID ( QLatin1String ( "datastorage" ) );
	s.setData("filename", filename);
	s.setData("document", document->getData());
	ControlServerSocket::instance()->sendToClients ( s.getjson(), m_sessionid );
}

StorageNotifierSocket::StorageNotifierSocket ( int sessionid ) : m_sessionid(sessionid) {}

void ControlServerSocket::disableSecureConnections() {
	m_disabledSecureConnections = true;
}
