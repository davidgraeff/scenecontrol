#include "SocketThread.h"
#include <qt4/QtCore/QDebug>

SocketThread::SocketThread( QWsSocket * wsSocket ) :
	socket( wsSocket )
{
	// Set this thread as parent of the socket
	// This will push the socket in the good thread when using moveToThread on the parent
	if ( socket )
		socket->setParent( this );

	// Move this thread object in the thread himsleft
	// Thats necessary to exec the event loop in this thread
	moveToThread( this );
}

SocketThread::~SocketThread()
{

}

void SocketThread::run()
{
	// Connecting the socket signals here to exec the slots in the new thread
	connect( socket, SIGNAL(frameReceived(QString)), this, SIGNAL(messageReceived(QString)) );
	connect( socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
	connect( socket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)) );

	// Launch the event loop to exec the slots
	exec();
}

void SocketThread::sendMessage( QString message )
{
	socket->write( message );
}

void SocketThread::processPong( quint64 elapsedTime )
{
	QDebug( "ping: " + QString::number(elapsedTime) + " ms" );
}

void SocketThread::socketDisconnected()
{
	// Prepare the socket to be deleted after last events processed
	socket->deleteLater();

	// finish the thread execution (that quit the event loop launched by exec)
	quit();
}
