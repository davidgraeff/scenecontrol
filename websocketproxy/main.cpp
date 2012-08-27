#include <QCoreApplication>
#include <QtDebug>

#include "Server.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Server server;
	if (!server.startWebsocket()) {
	    return -1;
	}

	return app.exec();
}
