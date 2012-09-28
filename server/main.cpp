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
#include "shared/utils/logging.h"
#include "shared/utils/paths.h"
#include "plugins/plugincontroller.h"
#include "scene/scenecontroller.h"
#include "libdatastorage/datastorage.h"
#include "libdatastorage/importexport.h"
#include "controlsocket/socket.h"

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
        printf("%s - %s - Usage:\n%s [CMDs]\n"
               "--export [PATH]: Export all documents from the database and store them in PATH or the working directory\n"
               "--import [PATH]: Import all documents from PATH or the working directory and store them in the database\n"
               "--overwrite: Overwrite existing documents while exporting or importing\n"
			   "--nossl: Disable secure connections. For debug purposes only!\n"
               "--help: This help text\n"
               "--version: Version information, parseable for scripts. Quits after output.\n",
               ABOUT_SERVICENAME, ABOUT_VERSION, argv[0]);
        return 0;
    }
    if (cmdargs.contains("--version")) {
        printf("%s\n%s\n", ABOUT_VERSION, ABOUT_LASTCOMMITDATE);
        return 0;
    }

    // Set up the datastorage object. All documents are accessable through the datastorage.
    DataStorage* datastorage = DataStorage::instance();

	if (cmdargs.contains("--export")) { // Export json documents from database
			int index = cmdargs.indexOf("--export");
			QString path = cmdargs.size()>index+1 ? QString::fromUtf8(cmdargs.at(index+1)) : QString();
			if (path.trimmed().isEmpty() || path.startsWith(QLatin1String("--")))
				path = QDir::currentPath();
			int processedDocuments = Datastorage::exportAsJSON(*datastorage, path, cmdargs.contains("--overwrite"));
			delete datastorage;
			if (cmdargs.contains("--overwrite"))
				qDebug() << processedDocuments << "Documents exported to" << path << "and overwritten existing";
			else
				qDebug() << processedDocuments << "Documents exported to" << path;
			return 0;
	} else if (cmdargs.contains("--import")) { // Import json documents from database
			int index = cmdargs.indexOf("--import");
			QString path = cmdargs.size()>index+1 ? QString::fromUtf8(cmdargs.at(index+1)) : QString();
			if (path.trimmed().isEmpty() || path.startsWith(QLatin1String("--")))
				path = QDir::currentPath();
			int processedDocuments = Datastorage::importFromJSON(*datastorage, path, cmdargs.contains("--overwrite"));
			delete datastorage;
			printf("Imported documents: %i\n", processedDocuments);
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
    setLogOptions("Server", ROOM_LOGTOCONSOLE, ROOM_LOGFILE);
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
    if (cmdargs.contains("--nossl")) {
		qWarning() << "SECURE CONNECTIONS ARE DISABLED. EVERYONE CAN CONTROL THIS SERVER NOW!";
		socket->disableSecureConnections();
	}

    // Write last start time to the log
    setup::writeLastStarttime();

    // CollectionController: Starts, stops collections
    // and hold references to running collections.
    CollectionController* collectioncontroller = CollectionController::instance();

    // PluginController: Start plugin processes and set up sockets for communication
    PluginController* plugins = PluginController::instance();

    // connect objects
    QObject::connect(datastorage, SIGNAL(doc_changed(const SceneDocument*)), plugins,  SLOT(doc_changed(const SceneDocument*)));
    QObject::connect(datastorage, SIGNAL(doc_removed(const SceneDocument*)), plugins, SLOT(doc_removed(const SceneDocument*)));	

	// Import json documents from install dir if no files are presend in the user storage dir
	bool success;
	QDir userdir = setup::dbuserdir(&success);
	if (!success) {
		qWarning()<<"Could not create user storage dir!";
		return -1;
	}
	if (userdir.entryList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot).size()==0) {
		Datastorage::VerifyImportDocument verifier;
		Datastorage::importFromJSON(*datastorage, setup::dbimportDir().absolutePath(), false, &verifier);
	}

	// load data storage and start plugin processes
	datastorage->load();
	plugins->scanPlugins();

	if (!cmdargs.contains("--no-event-loop") && plugins->valid()) { // Start event loop
		exitcode = qapp.exec();
	}
	// one last event processing to free all deleteLater objects
	qapp.processEvents();


	delete plugins;
	delete socket;
    delete datastorage;
    delete collectioncontroller;

    // close log file. only console log is possible from here on
    logclose();
	qDebug() << "Shutdown complete. Exitcode:" << exitcode;
    return exitcode;
}
