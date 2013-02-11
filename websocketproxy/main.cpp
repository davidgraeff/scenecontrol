#include <QCoreApplication>
#include <QtDebug>
#include "config.h"
#include "Server.h"

#define VAL(x) #x
#define STRINGIFY(x) VAL(x)

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	// commandline arguments
	QList<QByteArray> cmdargs;
	for (int i=0;i<argc;++i) cmdargs.append(QByteArray(argv[i]));
	
	// help text
	if (cmdargs.contains("--help") || cmdargs.contains("-h")) {
		printf("Usage:\n%s [CMDs]\n"
		"--port [port]: On which port should clients be able to connect for websocket functionality\n"
		"--server [host:port]: SceneServer SSL Host and Port\n"
		"--nossl: Disable secure connection to the SceneServer. For debug purposes only!\n"
		"--nowss: Disable secure connection to the Websocket client. Allow to connect via ws protocol instead of wss\n"
		"--help: This help text\n"
		"--version: Version information, parseable for scripts. Quits after output.\n", argv[0]);
		return 0;
	}
	if (cmdargs.contains("--version")) {
		printf("%s\n%s\n", ABOUT_VERSION, ABOUT_LASTCOMMITDATE);
		return 0;
	}
	
	int index = cmdargs.indexOf("--port");
	int listenport = (index!=-1 && cmdargs.size()>index+1) ? cmdargs.at(index+1).toInt() : ROOM_WEBSOCKETPROXY_LISTENPORT;
	index = cmdargs.indexOf("--server");
	QString sceneserver = (index!=-1 && cmdargs.size()>index+1) ? QString::fromUtf8(cmdargs.at(index+1)) : QLatin1String("127.0.0.1:" STRINGIFY(ROOM_LISTENPORT));
	
	qDebug() << "WebsocketProxy\n\tListeningport:" << listenport<<"\n\tSceneServer:"<<sceneserver<<"\n\tRaw Socket Security:"<<!cmdargs.contains("--nossl")<<"\n\tWebocket Security:"<<!cmdargs.contains("--nowss");
	
	Server server;
	if (!server.startWebsocket(sceneserver, listenport, cmdargs.contains("--nossl"), cmdargs.contains("--nowss"))) {
	    return -1;
	}

	return app.exec();
}
