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

static void catch_int(int )
{
    QCoreApplication::exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);

    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String(ROOM_SERVICENAME));
    qapp.setApplicationVersion(QLatin1String(ABOUT_VERSION));
    qapp.setOrganizationName(QLatin1String(ABOUT_ORGANIZATIONID));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	setLogOptions(ROOM_LOGTOCONSOLE,ROOM_LOGFILE);

    qInstallMsgHandler(roomMessageOutput);
    qDebug() << QCoreApplication::applicationName().toAscii().data() << ":" << QCoreApplication::applicationVersion().toAscii().data();

	QFile lockfile(QString(QLatin1String("%1/roomcontrolserver.pid")).arg(QDir::tempPath())); 
	if (lockfile.exists()) {
		qWarning() << "Lockfile"<<lockfile.fileName()<<"already exists. Only one instance at a time is allowed. If you are sure no other instance is running, remove the lockfile!";
		return -1;
	}
	lockfile.open(QIODevice::ReadWrite);
	lockfile.write("");

    ServiceController* services = new ServiceController();
#ifdef WITH_EXTERNAL
    network = new NetworkController();
    network->setServiceController(services);
    network->connect(services,SIGNAL(serviceSync(AbstractServiceProvider*)),SLOT(serviceSync(AbstractServiceProvider*)));
    network->connect(services,SIGNAL(statetrackerSync(AbstractStateTracker*)),SLOT(statetrackerSync(AbstractStateTracker*)));
    network->start();
#endif

    int r = 0;
    if (argc<=1 || strcmp("--shutdown", argv[1])!=0)
        r = qapp.exec();

	// remove lockfile
	lockfile.remove();
	
	#ifdef WITH_EXTERNAL
    qDebug() << "Shutdown: Network server";
    delete network;
    network = 0;
	#endif

    qDebug() << "Shutdown: Service Controller";
    delete services;
    services = 0;

    // restart program
    if (r == ROOM_RESTART_ON_CLOSE) {
        r = 0;
        if (argc>1 && strcmp("--norestart", argv[1])==0) {
            qDebug() << "Shutdown: Start another instance not allowed";
        } else {
            qDebug() << "Shutdown: Start another instance";
			logclose();
            QProcess::startDetached(QString::fromAscii(argv[0]));
			return 0;
        }
    }

	logclose();
	
    return r;
}
