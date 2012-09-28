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

Socket::~Socket()
{
}

Socket::Socket() {
    if (listen(QHostAddress::Any, ROOM_LISTENPORT)) {
        qDebug() << "SSL TCPSocket Server ready on port" << ROOM_LISTENPORT;
    }
}

static Socket* websocket_instance = 0;
Socket* Socket::instance()
{
    if (!websocket_instance)
        websocket_instance = new Socket();
    return websocket_instance;
}

void Socket::incomingConnection(int socketDescriptor)
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
				QSslError error(QSslError::SelfSignedCertificate, sslCert);
				expectedSslErrors.append(error);
				QSslError error2(QSslError::HostNameMismatch, sslCert);
				expectedSslErrors.append(error2);
				qDebug() << "Add client ssl certificate" << fileCert.fileName();
			}
		}
		else
		{
			qWarning() << fileCert.errorString();
		}
	}
	
	socket->ignoreSslErrors(expectedSslErrors);
	socket->startServerEncryption();
	qDebug() << "New connection" << socket->peerAddress();

	// Notify plugins of new session
	PluginController* pc = PluginController::instance();
	QMap<QString,PluginProcess*>::iterator i = pc->getPluginIterator();
	while (PluginProcess* plugin = pc->nextPlugin(i)) {
		plugin->session_change(socketDescriptor, true);
	}
}

void Socket::sslErrors ( const QList<QSslError> & errors ) {
	QList<QSslError> filteredErrors(errors);
	for (int i=filteredErrors.size()-1;i>=0;--i)
		if (filteredErrors[i].error() == QSslError::SelfSignedCertificate)
			filteredErrors.removeAt(i);
	if (filteredErrors.size())
		qWarning() << "SSL Errors" << filteredErrors;
}

void Socket::readyRead() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
    while (serverSocket->canReadLine()) {
		// Create a SceneDocument out of the raw data
        const QByteArray rawdata = serverSocket->readLine();
        if (!rawdata.length())
            continue;
        QVariant v =JSON::parse(rawdata);
        if (v.isNull()) {
			qWarning()<<"Server Socket: Failed to parse json" << rawdata;
			serverSocket->write("{\"componentid_\":\"server\",\"type_\":\"serverresponse\", \"id_\":\"no_json\", \"msg\":\"Failed to parse json\"}\n");
			continue;
		}
		
		// Analyse the scene document: We only accept TypeExecution
		SceneDocument doc(v.toMap());
		if ( !doc.checkType ( SceneDocument::TypeExecution ) ) {
			serverSocket->write("{\"componentid_\":\"server\",\"type_\":\"serverresponse\", \"id_\":\"no_execution_type\", \"msg\":\"No execution type\"}\n");
			continue;
		}
		
		const int sessionid = serverSocket->socketDescriptor();
		
		// Handle the case where the incoming scene document is dedicated for a plugin
		if ( doc.componentID() != QLatin1String ( "server" ))
		{
			// Try to execute the SceneDocument. Find a plugin for it first.
			PluginProcess* plugin = PluginController::instance()->getPlugin ( doc.componentUniqueID() );
			if ( !plugin )
			{
				serverSocket->write("{\"componentid_\":\"server\",\"type_\":\"serverresponse\", \"id_\":\"plugin_not_found\", \"msg\":\"Plugin not found\"}\n");
				continue;
			}
			// Call the remote method of the plugin
			plugin->callQtSlot ( doc.getData(), QByteArray(), sessionid );
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
		
		else if ( doc.isMethod ( "removeDocument" ) )
		{
			DataStorage::instance()->removeDocument(SceneDocument(v.toMap().value(QLatin1String("doc")).toMap()));
		}
		
		else if ( doc.isMethod ( "changeDocument" ) )
		{
			DataStorage::instance()->storeDocument(SceneDocument(v.toMap().value(QLatin1String("doc")).toMap()), true);
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
			s.setData("notifierstate", true);
			sendToClients ( s.getjson(), sessionid );
		}
		
		else if ( doc.isMethod ( "runcollection" ) )
		{
			CollectionController::instance()->requestExecutionByCollectionId ( doc.sceneid() );
		}

		else if ( doc.isMethod ( "version" ) )
		{
			SceneDocument s = SceneDocument::createNotification ( "version" );
			s.setData ( "version", QLatin1String ( ABOUT_VERSION ) );
			s.setComponentID ( QLatin1String ( "server" ) );
			sendToClients ( s.getjson(), sessionid );
		} else
		
		{
			serverSocket->write("{\"componentid_\":\"server\",\"type_\":\"serverresponse\", \"id_\":\"method_not_known\", \"msg\":\"Method not known\"}\n");
		}
    }
}

void Socket::socketDisconnected() {
    QSslSocket *socket = (QSslSocket *)sender();
    const int socketDescriptor = socket->socketDescriptor();
    m_sockets.remove(socketDescriptor);
    socket->deleteLater();

    // Notify plugins of finished session
    PluginController* pc = PluginController::instance();
    QMap<QString,PluginProcess*>::iterator i = pc->getPluginIterator();
    while (PluginProcess* plugin = pc->nextPlugin(i)) {
        plugin->session_change(socketDescriptor, false);
    }

	if (m_notifiers.contains(socketDescriptor)) {
		delete m_notifiers.take(socketDescriptor);
	}
	
    qDebug() << "socket closed" << socketDescriptor << socket->errorString() << socket->error();
}

void Socket::sendToClients(const QByteArray& rawdata, int sessionid) {
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


StorageNotifierSocket* Socket::notifier ( int sessionid ) {
	StorageNotifierSocket* s = m_notifiers[sessionid];
	if (!s) {
		s = new StorageNotifierSocket(sessionid);
		m_notifiers[sessionid] = s;
	}
	return s;
}

void StorageNotifierSocket::documentChanged ( const QString& filename, SceneDocument* document ) {
	// Create the response
	SceneDocument s = SceneDocument::createNotification ( "documentChanged" );
	s.setComponentID ( QLatin1String ( "datastorage" ) );
	s.setData("filename", filename);
	s.setData("document", document->getData());
	Socket::instance()->sendToClients ( s.getjson(), m_sessionid );
}

void StorageNotifierSocket::documentRemoved ( const QString& filename, SceneDocument* document ) {
	// Create the response
	SceneDocument s = SceneDocument::createNotification ( "documentRemoved" );
	s.setComponentID ( QLatin1String ( "datastorage" ) );
	s.setData("filename", filename);
	s.setData("document", document->getData());
	Socket::instance()->sendToClients ( s.getjson(), m_sessionid );
}

StorageNotifierSocket::StorageNotifierSocket ( int sessionid ) : m_sessionid(sessionid) {}
