#pragma once

#include <QThread>

#include "QWsSocket.h"

class SocketThread : public QThread
{
	Q_OBJECT

public:
	SocketThread( QWsSocket * wsSocket );
	~SocketThread();

	QWsSocket * socket;
	void run();

private slots:
	void sendMessage( QString message );
	void processPong( quint64 elapsedTime );
	void socketDisconnected();

signals:
	void messageReceived( QString frame );

private:
	
};
