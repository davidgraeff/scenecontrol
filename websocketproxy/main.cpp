#include <QApplication>
#include <QtDebug>

#include "Server.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Server server;
	if (!server.connectToSceneServer()) {
	    return -1;
	}

	return app.exec();
}
