/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <QCoreApplication>
#include <QProcess>
#include "servicecontroller.h"
#include "externalcontrol/httpserver.h"
#include <QSettings>
#include "config.h"
#include <qtextcodec.h>
#include "logging.h"
#include "backups.h"
#include "collections.h"
#include <stdio.h>
#include "sessioncontroller.h"
#include "plugincontroller.h"

bool exitByConsoleCommand = false;

static void catch_int(int )
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    exitByConsoleCommand = true;
	printf("\n");
    QCoreApplication::exit(0);
}

int main(int argc, char *argv[])
{
    // commandline arguments
    QSet<QByteArray> cmdargs;
    for (int i=0;i<argc;++i) cmdargs.insert(QByteArray(argv[i]));

    // help text
    if (cmdargs.contains("--help")) {
        printf("%s - %s\n%s [--no-restart] [--no-event-loop] [--no-network] [--observe-service-dir] [--ignore-lock] [--help] [--version]\nLockfile: %s\n"
               "--no-restart: Do not restart after exit\n--no-event-loop: Shutdown after initialisation\n--no-network: Do not load network objects\n"
			   "--observe-service-dir: Reload changed files in the service directory\n--ignore-lock: Try to remove lock and acquire lock file\n"
			   "--help: This help text\n--version: Version information, parseable for scripts\n",
               ROOM_SERVICENAME, ABOUT_VERSION, argv[0],
               QString(QLatin1String("%1/roomcontrolserver.pid")).arg(QDir::tempPath()).toUtf8().constData());
        return 0;
    }
    if (cmdargs.contains("--version")) {
        printf("%s\n", ABOUT_VERSION);
        return 0;
    }

    //set up signal handlers to exit on CTRL+C
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);

    // Qt Application Object, choose language codec
    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String(ROOM_SERVICENAME));
    qapp.setApplicationVersion(QLatin1String(ABOUT_VERSION));
    qapp.setOrganizationName(QLatin1String(ABOUT_ORGANIZATIONID));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    // set uo log options and message handler and print out a first message
    setLogOptions(ROOM_LOGTOCONSOLE, ROOM_LOGFILE);
    qInstallMsgHandler(roomMessageOutput);
    qDebug() << QString(QLatin1String("%1 (%2) - Pid: %3")).
    arg(QCoreApplication::applicationName()).
    arg(QCoreApplication::applicationVersion()).
    arg(QCoreApplication::applicationPid());

    // create lock file to allow only one instance of the server process
    QFile lockfile(QString(QLatin1String("%1/roomcontrolserver.pid")).arg(QDir::tempPath()));
    if (lockfile.exists() && (!cmdargs.contains("--ignore-lock") || !lockfile.remove())) {
        qWarning() << "Lockfile"<<lockfile.fileName()<<"already exists. Only one instance at a time is allowed. If you are sure no other instance is running, remove the lockfile!";
        return -1;
    }
    lockfile.open(QIODevice::ReadWrite|QIODevice::Append);
    lockfile.write(QByteArray::number(QCoreApplication::applicationPid()));

    // service controller (implements AbstractServer)
    ServiceController* services = new ServiceController();
    PluginController* plugins = new PluginController(services);
	plugins->registerPluginFromObject(services);


    // backups
    Backups* backups = new Backups();
    backups->connectToServer(services);
    plugins->registerPluginFromObject(backups);

    // collections
    Collections* collections = new Collections();
    collections->connectToServer(services);
    collections->setServiceController(services);
    plugins->registerPluginFromObject(collections);
    collections->connect(services, SIGNAL(dataReady()), collections, SLOT(dataReady()));
    collections->connect(services, SIGNAL(dataSync(QVariantMap,QString)), collections, SLOT(dataSync(QVariantMap,QString)));
    collections->connect(services, SIGNAL(eventTriggered(QString,QString)), collections, SLOT(eventTriggered(QString,QString)));
    services->connect(collections, SIGNAL(instanceExecute(QString, QString)), services, SLOT(executeActionByUID(QString, QString)));

	SessionController* sessions = SessionController::instance(true);
	sessions->connectToServer(services);
	plugins->registerPluginFromObject(sessions);
	services->connect(sessions,SIGNAL(sessionBegin(QString)),services,SLOT(sessionBegin(QString)));
	services->connect(sessions,SIGNAL(sessionFinished(QString,bool)),services,SLOT(sessionFinished(QString,bool)));

    services->load(cmdargs.contains("--observe-service-dir"));
	plugins->initializePlugins();

    // network
#ifdef WITH_EXTERNAL
    HttpServer* httpserver = 0;
    if (!cmdargs.contains("--no-network")) {
        httpserver = new HttpServer();
		httpserver->connect(services,SIGNAL(dataSync(QVariantMap,QString)), httpserver, SLOT(dataSync(QVariantMap,QString)));
		httpserver->connect(sessions,SIGNAL(sessionBegin(QString)),httpserver,SLOT(sessionBegin(QString)));
		httpserver->connect(sessions,SIGNAL(sessionFinished(QString,bool)),httpserver,SLOT(sessionFinished(QString,bool)));
		httpserver->connect(httpserver,SIGNAL(dataReceived(QVariantMap,QString)),services,SLOT(changeService(QVariantMap,QString)));
        httpserver->start();
    }
#endif

    int exitcode = 0;
    if (!cmdargs.contains("--no-event-loop"))
        exitcode = qapp.exec();

#ifdef WITH_EXTERNAL
    qDebug() << "Shutdown: Network server";
    delete httpserver;
    httpserver = 0;
#endif

    qDebug() << "Shutdown: Service Controller";
	plugins->deregisterPluginFromObject(sessions, services);
    plugins->deregisterPluginFromObject(collections, services);
    plugins->deregisterPluginFromObject(backups, services);
	delete sessions;
    delete collections;
    delete backups;
    delete services;
	delete plugins;
    services = 0;

    // remove lockfile
    lockfile.remove();

    // close log file. only console log is possible from here on
    logclose();

    // restart program
    if (ROOM_RESTART_ON_CLOSE) {
        if (exitByConsoleCommand) {
			qDebug() << "Shutdown: Console shutdown. No restart allowed!";
		} else if (cmdargs.contains("--no-restart")) {
            qDebug() << "Shutdown: Start another instance not allowed!";
        } else if (exitcode == 0) {
            qDebug() << "Shutdown: Start another instance";
			int rtry = 0;
			for (int i=0;i<argc;++i) {
				QString arg = QString::fromAscii(argv[i]);
				if (arg.startsWith(QLatin1String("--restart-try="))) rtry = arg.mid(strlen("--restart-try=")).toInt();
			}
			QString cmd = QString::fromAscii(argv[0]) + QLatin1String(" --restart-try=") + QString::number(rtry);
            QProcess::startDetached(cmd);
        }
    }

    return exitcode;
}
