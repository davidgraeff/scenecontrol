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
#include "plugincontroller.h"
#include "socket.h"
#include "collectioncontroller.h"
#include "database.h"
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
    QCoreApplication::exit(0);
}

int main(int argc, char *argv[])
{
    // commandline arguments
    QList<QByteArray> cmdargs;
    for (int i=0;i<argc;++i) cmdargs.append(QByteArray(argv[i]));

    // help text
    if (cmdargs.contains("--help")) {
        printf("%s - %s\n%s [CMDs]\n"
               "--no-restart: Do not restart after exit\n"
               "--no-event-loop: Shutdown after initialisation\n"
               "--export [PATH]: Export all documents from the database and store them in PATH or the working directory\n"
               "--import [PATH]: Import all documents from PATH or the working directory and store them in the database\n"
               "--no-autoload-plugins: Only start the server and no plugin processes\n"
               "--help: This help text\n"
               "--version: Version information, parseable for scripts. Quits after output.\n",
               ROOM_SERVICENAME, ABOUT_VERSION, argv[0]);
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
    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String(ROOM_SERVICENAME));
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
    Database* database = Database::instance();

    // CollectionController: Starts, stops collections
    // and hold references to running collections.
    CollectionController* collectioncontroller = CollectionController::instance();

    // PluginController: Start plugin processes and set up sockets for communication
    PluginController* plugins = PluginController::instance();

    // connect objects
    plugins->connect(database, SIGNAL(Event_add(QString,QVariantMap)), plugins,  SLOT(Event_add(QString,QVariantMap)));
    plugins->connect(database, SIGNAL(Event_remove(QString)), plugins, SLOT(Event_remove(QString)));
    plugins->connect(database, SIGNAL(settings(QString,QString,QVariantMap)), plugins, SLOT(settings(QString,QString,QVariantMap)));
    collectioncontroller->connect(socket, SIGNAL(requestExecution(QVariantMap,int)), collectioncontroller, SLOT(requestExecution(QVariantMap,int)));
    collectioncontroller->connect(database, SIGNAL(dataOfCollection(QList<QVariantMap>,QList<QVariantMap>,QString)), collectioncontroller, SLOT(dataOfCollection(QList<QVariantMap>,QList<QVariantMap>,QString)));

    int exitcode = 0;
    exitcode |= database->connectToDatabase()?0:-2;

    // Export json documents from database if requested
    {
        int index = cmdargs.indexOf("--export");
        if (index!=-1) {
            QString path = cmdargs.size()>=index ? QString::fromUtf8(cmdargs.at(index+1)) : QString();
            if (path.trimmed().isEmpty() || path.startsWith(QLatin1String("--")))
                path = QDir::currentPath();
            database->exportAsJSON(path);
        }
    }

    // Import json documents from database if requested
    {
        int index = cmdargs.indexOf("--import");
        if (index!=-1) {
            QString path = cmdargs.size()>=index ? QString::fromUtf8(cmdargs.at(index+1)) : QString();
            if (path.trimmed().isEmpty() || path.startsWith(QLatin1String("--")))
                path = QDir::currentPath();
            database->importFromJSON(path);
        }
    }
    
    // Start event loop (if --no-event-loop is not set)
    if (!exitcode && !cmdargs.contains("--no-event-loop")) {
        // Start plugin processes
        if (!cmdargs.contains("--no-autoload-plugins"))
            plugins->startplugins();
        // start change listeners
        database->startChangeListenerEvents();
        database->startChangeListenerSettings();
        exitcode = qapp.exec();
    }

    qDebug() << "Shutdown...";
    delete socket;
    delete collectioncontroller;
    delete plugins;
    delete database;

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
            int retry = 0;
            for (int i=0;i<argc;++i) {
                QString arg = QString::fromAscii(argv[i]);
                if (arg.startsWith(QLatin1String("--restart-try=")))
					retry = arg.mid(strlen("--restart-try=")).toInt();
            }
            if (retry > 3) {
				qWarning() << "Retry maximum reached";
				return exitcode;
            }
            QString cmd = QString::fromAscii(argv[0]) + QLatin1String(" --restart-try=") + QString::number(retry);
            QProcess::startDetached(cmd);
        }
    }

    return exitcode;
}
