#ifndef HEADER_WebSocketServer
#define HEADER_WebSocketServer

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"

class Server : public QObject
{
	Q_OBJECT

public:
	Server();
	~Server();
    bool startWebsocket(const QString& sceneserver, int listenport, bool disableSecureConnection);

public Q_SLOTS:
	// Debug output slots
	void sslErrors ( const QList<QSslError> & errors );
	void pong( quint64 elapsedTime );
	// new client connection slot
 	void newClientConnection();
	// message passing
	void processClientMessage( const QByteArray& message );
	void processServerMessage();
	// disconnection
	void serverDisconnected();
	void clientDisconnected();
    void serverError ( QAbstractSocket::SocketError );
private:
	QWsServer * m_server;
	QMap<QSslSocket*, QWsSocket*> m_server_to_client;
	QMap<QWsSocket*, QSslSocket*> m_client_to_server;
	QSslKey readKey(const QString& fileKeyString);
	QSslCertificate readCertificate(const QString& filename);
	QUrl m_sceneserver;
	bool m_disableSecureConnection;
};

#endif
