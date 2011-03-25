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
#include <qprocess.h>
#include "servicecontroller.h"
#include "externalcontrol/httpserver.h"
#include <QSettings>
#include "config.h"
#include <qtextcodec.h>
#include "logging.h"
#include "backups.h"
#include "collections.h"
#include "serverstate.h"
#include <stdio.h>

static void catch_int(int )
{
    QCoreApplication::exit(0);
}

int main(int argc, char *argv[])
{
    // commandline arguments
    QSet<QByteArray> cmdargs;
    for (int i=0;i<argc;++i) cmdargs.insert(QByteArray(argv[i]));

    // help text
    if (cmdargs.contains("--help")) {
        printf("%s - %s\n%s [--no-restart] [--no-event-loop] [--no-network] [--observe-service-dir] [--help] [--version]\nLockfile: %s\n"
               "--no-restart: Do not restart after exit\n",
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
    qDebug() << QCoreApplication::applicationName().toAscii().data() << ":" << QCoreApplication::applicationVersion().toAscii().data();

    // create lock file to allow only one instance of the server process
    QFile lockfile(QString(QLatin1String("%1/roomcontrolserver.pid")).arg(QDir::tempPath()));
    if (lockfile.exists()) {
        qWarning() << "Lockfile"<<lockfile.fileName()<<"already exists. Only one instance at a time is allowed. If you are sure no other instance is running, remove the lockfile!";
        return -1;
    }
    lockfile.open(QIODevice::ReadWrite);
    lockfile.write("");

    // service controller (implements AbstractServer)
    ServiceController* services = new ServiceController();

    // backups
    Backups* backups = new Backups();
    backups->connectToServer(services);
    services->useServerObject(backups);

    // collections
    Collections* collections = new Collections();
    collections->connectToServer(services);
    services->useServerObject(collections);
	collections->connect(services, SIGNAL(dataReady()), collections, SLOT(dataReady()));
	collections->connect(services, SIGNAL(dataSync(QVariantMap,bool,QString)), collections, SLOT(dataSync(QVariantMap,bool,QString)));
	collections->connect(services, SIGNAL(eventTriggered(QString)), collections, SLOT(eventTriggered(QString)));
	services->connect(collections, SIGNAL(instanceExecute(QString)), services, SLOT(executeService(QString)));

    // serverstate
    ServerState* serverstate = new ServerState();
    serverstate->connectToServer(services);
    services->useServerObject(serverstate);

	services->load(cmdargs.contains("--observe-service-dir"));
	
    // network
#ifdef WITH_EXTERNAL
    NetworkController* network = 0;
    if (!cmdargs.contains("--no-network")) {
        network = new NetworkController();
        network->setServiceController(services);
        network->start();
    }
#endif

    int exitcode = 0;
    if (!cmdargs.contains("--no-event-loop"))
        exitcode = qapp.exec();

    // remove lockfile
    lockfile.remove();

#ifdef WITH_EXTERNAL
    qDebug() << "Shutdown: Network server";
    delete network;
    network = 0;
#endif

    qDebug() << "Shutdown: Service Controller";
    services->removeServerObject(serverstate);
    services->removeServerObject(collections);
    services->removeServerObject(backups);
    delete serverstate;
    delete collections;
    delete backups;
    delete services;
    services = 0;

    // close log file. only console log is possible from here on
    logclose();

    // restart program
    if (ROOM_RESTART_ON_CLOSE) {
        if (cmdargs.contains("--no-restart")) {
            qDebug() << "Shutdown: Start another instance not allowed";
        } else {
            qDebug() << "Shutdown: Start another instance";
            QProcess::startDetached(QString::fromAscii(argv[0]));
        }
    }

    return exitcode;
}
