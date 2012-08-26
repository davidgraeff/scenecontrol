#include "Server.h"

Server::Server()
{
	int port = 1337;
    server = new QWsServer( this );
	if ( ! server->listen( QHostAddress::Any, port ) )
	{
		qDebug() << "Error: Can't launch server";
		qDebug() << "QWsServer error :" << server->errorString();
	}
	else
	{
		qDebug() << "Server is listening port " + QString::number(port);
	}
	connect( server, SIGNAL(newConnection()), this, SLOT(processNewConnection()) );
}

Server::~Server()
{
}

void Server::processNewConnection()
{
	qDebug() << "Client connected";

	// Get the connecting socket
	QWsSocket * socket = server->nextPendingConnection();

	// Create a new thread and giving to him the socket
	SocketThread * thread = new SocketThread( socket );
	
	// connect for message broadcast
	connect( socket, SIGNAL(frameReceived(QString)), this, SIGNAL(broadcastMessage(QString)) );
	//connect( this, SIGNAL(broadcastMessage(QString)), thread, SLOT(sendMessage(QString)) );

	// connect for message display in log
	connect( socket, SIGNAL(frameReceived(QString)), this, SLOT(processWSMessage(QString)) );

	// Starting the thread
	thread->start();
}

void Server::processWSMessage( QString message )
{
	// Just display in log the message received by a socket
	qDebug() << QString::fromUtf8( message.toStdString().c_str() );
}	
bool Server::connectToSceneServer() {}
