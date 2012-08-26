#ifndef HEADER_WebSocketServer
#define HEADER_WebSocketServer

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"
#include "SocketThread.h"

class Server : public QObject
{
	Q_OBJECT

public:
	Server();
	~Server();
	bool connectToSceneServer();

public slots:
	void processNewConnection();
	void processWSMessage( QString message );

signals:
	void broadcastWSMessage( QString message );

private:
	QWsServer * server;
};

#endif
