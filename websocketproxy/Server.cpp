#include "Server.h"
#include "config.h"
#include <shared/utils/paths.h>
#include <shared/jsondocuments/json.h>
#include <shared/jsondocuments/scenedocument.h>
Server::Server() : m_server(0) {}

Server::~Server() {}

void Server::newClientConnection()
{
	// Get the client socket
	QWsSocket * clientsocket = m_server->nextPendingConnection();

	// send ping
	clientsocket->ping();

	// create server socket
    QSslSocket *serversocket = new QSslSocket;
	connect(serversocket, SIGNAL(sslErrors (QList<QSslError>)), this, SLOT(sslErrors (QList<QSslError>)));
	serversocket->ignoreSslErrors();
	serversocket->setProtocol(QSsl::SslV3);
	serversocket->setPeerVerifyMode(QSslSocket::VerifyNone);

	// Add client key: Currently not used
	QFile fileKey(setup::certificateFile("clients/websocketproxy.key"));
	if (fileKey.open(QIODevice::ReadOnly))
	{
		QByteArray key = fileKey.readAll();
		fileKey.close();
		QSslKey sslKey(key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "1234");
		if (key.isNull()) {
			qWarning() << "key invalid";
		} else
			serversocket->setPrivateKey(sslKey);
	}
	else
	{
		qWarning() << fileKey.errorString();
	}

	// Set public certificate
	QFile fileCert(setup::certificateFile("clients/websocketproxy.crt"));
	if (fileCert.open(QIODevice::ReadOnly))
	{
		QByteArray cert = fileCert.readAll();
		fileCert.close();
		QSslCertificate sslCert(cert);
		if (sslCert.isNull()) {
			qWarning() << "sslCert invalid";
		} else
			serversocket->setLocalCertificate(sslCert);
	}
	else
	{
		qWarning() << fileCert.errorString();
	}
	
	// Add public certificate of the server to the trusted hosts
	QFile fileCertServer(setup::certificateFile("server.crt"));
	if (fileCertServer.open(QIODevice::ReadOnly))
	{
		QByteArray cert = fileCertServer.readAll();
		fileCertServer.close();
		QSslCertificate sslCert(cert);
		if (sslCert.isNull()) {
			qWarning() << "sslCert invalid";
		} else
			serversocket->addCaCertificate(sslCert);
	}
	else
	{
		qWarning() << fileCert.errorString();
	}

	// add to lists
	m_server_to_client.insert(serversocket,clientsocket);
	m_client_to_server.insert(clientsocket,serversocket);
	
	// Connect signals
	connect( clientsocket, SIGNAL(frameReceivedText(QByteArray)), this, SLOT(processClientMessage(QByteArray)) );
	connect( clientsocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()) );
	connect( clientsocket, SIGNAL(pong(quint64)), this, SLOT(pong(quint64)) );
	
	connect(serversocket, SIGNAL(readyRead()), this, SLOT(processServerMessage()));
	connect(serversocket, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
	connect(serversocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));

	// Now try to connect to the server: If it fails the disconnect signal will clean up
	if (m_disableSecureConnection)
		serversocket->connectToHost(m_sceneserver.host(), m_sceneserver.port());
	else
		serversocket->connectToHostEncrypted(m_sceneserver.host(), m_sceneserver.port());
}


void Server::sslErrors ( const QList<QSslError> & errors ) {
	QList<QSslError> filteredErrors(errors);
	for (int i=filteredErrors.size()-1;i>=0;--i)
		if (filteredErrors[i].error() == QSslError::SelfSignedCertificate || filteredErrors[i].error() == QSslError::HostNameMismatch)
			filteredErrors.removeAt(i);
	if (filteredErrors.size())
		qWarning() << "SSL Errors" << filteredErrors;
}

void Server::pong( quint64 elapsedTime )
{
	QWsSocket *clientSocket = (QWsSocket *)sender();
	qDebug().nospace() << "Connected SceneServer " << qPrintable(m_sceneserver.host()) << ":" << m_sceneserver.port() << " to client " << qPrintable(clientSocket->hostAddress())<<":"<<clientSocket->hostPort()
	<< " with " << elapsedTime << "ms ping time" ;
}

void Server::processServerMessage() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
    while (serverSocket->canReadLine()) {
        const QByteArray message = serverSocket->readLine();
        if (!message.length())
            continue;
		bool error;
		JSON::parse(message, &error);
		if (error) {
			qWarning()<<"Invalid document server->client";
			continue;
		}

		QWsSocket* clientSocket = m_server_to_client.value(serverSocket);
		Q_ASSERT(clientSocket);
		clientSocket->writeText(message);
    }
}

void Server::processClientMessage( const QByteArray& message )
{
	QWsSocket *clientSocket = (QWsSocket *)sender();
	if (!message.length())
		return;
	bool error;
	JSON::parse(message, &error);
	if (error) {
		qWarning()<<"Invalid document client->server";
		// send message to client
		SceneDocument doc;
		doc.setData("error", true);
		doc.setData("errormsg", "Invalid document");
		clientSocket->writeText(doc.getjson());
		return;
	}

	QSslSocket* serverSocket = m_client_to_server.value(clientSocket);
	Q_ASSERT(serverSocket);
	serverSocket->write(message+"\n");
}	

void Server::serverDisconnected() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
	// remove from lists
    QWsSocket* clientSocket = m_server_to_client.take(serverSocket);
	if (!clientSocket)
		return;
	m_client_to_server.remove(clientSocket);
	// send message to client
	SceneDocument doc;
	doc.setData("error", true);
	doc.setData("errormsg", "Server disconnected");
	clientSocket->writeText(doc.getjson());
	// delete later
    serverSocket->deleteLater();
	clientSocket->deleteLater();
	// message
    qDebug() << "Server closed the connection" << clientSocket->hostAddress();
}

void Server::serverError ( QAbstractSocket::SocketError ) {
    QSslSocket *serverSocket = (QSslSocket *)sender();
	// remove from lists
    QWsSocket* clientSocket = m_server_to_client.take(serverSocket);
	if (!clientSocket)
		return;
	m_client_to_server.remove(clientSocket);
	// send message to client
	SceneDocument doc;
	doc.setData("error", true);
	doc.setData("errormsg", "Server error: "+serverSocket->errorString().toUtf8());
	clientSocket->writeText(doc.getjson()+"\n");
	// delete later
    serverSocket->deleteLater();
	clientSocket->deleteLater();
	// message
    qDebug() << "Server connection failed" << clientSocket->hostAddress() << serverSocket->errorString() << serverSocket->error();
}

void Server::clientDisconnected() {
    QWsSocket *clientSocket = (QWsSocket *)sender();
	// remove from lists
    QSslSocket* serverSocket = m_client_to_server.take(clientSocket);
	if (!serverSocket)
		return;
	m_server_to_client.remove(serverSocket);
	// delete later
    serverSocket->deleteLater();
	clientSocket->deleteLater();
	// message
    qDebug() << "Client closed the connection" << clientSocket->hostAddress() << clientSocket->errorString() << clientSocket->error();
}

bool Server::startWebsocket(const QString& sceneserver, int listenport, bool disableSecureConnection) {
	m_sceneserver = QLatin1String("tcp://") + sceneserver;
	m_disableSecureConnection = disableSecureConnection;
	delete m_server;
    m_server = new QWsServer( this );
	if ( ! m_server->listen( QHostAddress::Any, listenport ) )
	{
		qWarning() << "Error: Can't launch server";
		qWarning() << "QWsServer error :" << m_server->errorString();
		return false;
	}
	
	connect( m_server, SIGNAL(newConnection()), this, SLOT(newClientConnection()) );
	return true;
}
