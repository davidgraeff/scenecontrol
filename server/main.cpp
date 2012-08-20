/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2012  David Gräff

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

#include "config.h"
#include "logging.h"
#include "plugins/plugincontroller.h"
#include "execute/collectioncontroller.h"
#include "execute/executerequest.h"
#include "libdatastorage/datastorage.h"
#include "libdatastorage/importexport.h"
#include "socket.h"
#include "paths.h"

#include <stdio.h>
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <qtextcodec.h>
#include <QCoreApplication>
#include <QProcess>
#include <QSettings>
#include <QDebug>

bool exitByConsoleCommand = false;

static void catch_int(int )
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    exitByConsoleCommand = true;
    printf("\n");
	PluginController* plugins = PluginController::instance();
	plugins->waitForPluginsAndExit();
	DataStorage::instance()->unload();
}

int main(int argc, char *argv[])
{
    // commandline arguments
    QList<QByteArray> cmdargs;
    for (int i=0;i<argc;++i) cmdargs.append(QByteArray(argv[i]));

    // help text
    if (cmdargs.contains("--help")) {
        printf("%s - %s\n%s [CMDs]\n"
               "--export [PATH]: Export all documents from the database and store them in PATH or the working directory\n"
               "--import [PATH]: Import all documents from PATH or the working directory and store them in the database\n"
               "--no-event-loop: Shutdown after initialisation\n"
               "--help: This help text\n"
               "--version: Version information, parseable for scripts. Quits after output.\n",
               ABOUT_SERVICENAME, ABOUT_VERSION, argv[0]);
        return 0;
    }
    if (cmdargs.contains("--version")) {
        printf("%s\n%s\n", ABOUT_VERSION, ABOUT_LASTCOMMITDATE);
        return 0;
    }

    //set up signal handlers to exit on CTRL+C
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);

    // Qt Application; choose language codec
    int exitcode = 0;
    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String(ABOUT_SERVICENAME));
    qapp.setApplicationVersion(QLatin1String(ABOUT_VERSION));
    qapp.setOrganizationName(QLatin1String(ABOUT_ORGANIZATIONID));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    // Set up log options and message handler and print out a first message
    setLogOptions(ROOM_LOGTOCONSOLE, ROOM_LOGFILE);
    qInstallMsgHandler(roomMessageOutput);
    qDebug() << QString(QLatin1String("%1 (%2) - Pid: %3")).
    arg(QCoreApplication::applicationName()).
    arg(QCoreApplication::applicationVersion()).
    arg(QCoreApplication::applicationPid());

    // create network socket for controlling the server
    Socket* socket = Socket::instance();
    if (!socket->isListening()) {
        qWarning() << "TCP Socket initalizing failed. Maybe another instance is already running";
        delete socket;
        return -1;
    }

    // Write last start time to the log
    setup::writeLastStarttime();

    // Set up the database connection. All events, actions, configurations are hold within a database
    DataStorage* datastorage = DataStorage::instance();

    // CollectionController: Starts, stops collections
    // and hold references to running collections.
    CollectionController* collectioncontroller = CollectionController::instance();

    // PluginController: Start plugin processes and set up sockets for communication
    PluginController* plugins = PluginController::instance();

	// ExecuteRequest: Singleton to execute requests
	ExecuteRequest* executeRequests = ExecuteRequest::instance();
	
    // connect objects
    QObject::connect(datastorage, SIGNAL(doc_changed(SceneDocument)), plugins,  SLOT(Event_add(QString,QVariantMap)));
    QObject::connect(datastorage, SIGNAL(doc_removed(SceneDocument)), plugins, SLOT(Event_remove(QString)));
    QObject::connect(socket, SIGNAL(requestExecution(QVariantMap,int)), executeRequests, SLOT(requestExecution(QVariantMap,int)));

	// connect to the database
    datastorage->load();
	plugins->scanPlugins();

	if (cmdargs.contains("--export")) { // Export json documents from database
			int index = cmdargs.indexOf("--export");
			QString path = cmdargs.size()>=index ? QString::fromUtf8(cmdargs.at(index+1)) : QString();
			if (path.trimmed().isEmpty() || path.startsWith(QLatin1String("--")))
				path = QDir::currentPath();
			Datastorage::exportAsJSON(*datastorage, path);
	} else if (cmdargs.contains("--import")) { // Import json documents from database
			int index = cmdargs.indexOf("--import");
			QString path = cmdargs.size()>=index ? QString::fromUtf8(cmdargs.at(index+1)) : QString();
			if (path.trimmed().isEmpty() || path.startsWith(QLatin1String("--")))
				path = QDir::currentPath();
			Datastorage::importFromJSON(*datastorage, path);
	} else if (!cmdargs.contains("--no-event-loop") && plugins->valid()) { // Start event loop
        exitcode = qapp.exec();
		// one last event processing to free all deleteLater objects
		qapp.processEvents();
    }

	delete plugins;
	delete socket;
    delete datastorage;
    delete collectioncontroller;
	delete executeRequests;

    // close log file. only console log is possible from here on
    logclose();
	qDebug() << "Shutdown complete. Exitcode:" << exitcode;
    return exitcode;
}
