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
#include <QDBusError>
#define __FUNCTION__ __FUNCTION__
//#define __TIMESTAMP__ __TIMESTAMP__
#include <qprocess.h>
#include "networkcontroller.h"
#include "servicecontroller.h"
#include "shared/abstractserviceprovider.h"
#include <QSettings>
#include "config.h"
#include <qtextcodec.h>
#include <time.h>

NetworkController* network = 0;

void myMessageOutput(QtMsgType type, const char *msg)
{
    if (network) network->log(msg);
    time_t rawtime;
    tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[%2d:%02d] %s\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "[%2d:%02d] \033[33mWarning: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[%2d:%02d] \033[31mCritical: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[%2d:%02d] \033[31mFatal: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        abort();
    }
}

/* first, here is the signal handler */
static void catch_int(int )
{
    /* re-set the signal handler again to catch_int, for next time */
//     signal(SIGINT, catch_int);
//     signal(SIGTERM, catch_int);
    QCoreApplication::exit(0);
}


int main(int argc, char *argv[])
{
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);

    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String(ABOUT_PRODUCT));
    qapp.setApplicationVersion(QLatin1String(ABOUT_VERSION));
    qapp.setOrganizationName(QLatin1String(ABOUT_ORGANIZATIONID));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QDir::home();
    QDir m_savedir = QFileInfo ( QSettings().fileName() ).absoluteDir();
    m_savedir.mkpath ( m_savedir.absolutePath() );
    m_savedir.mkdir ( QLatin1String ( ABOUT_PRODUCT ) );
    m_savedir.cd ( QLatin1String ( ABOUT_PRODUCT ) );

    qapp.setProperty("settingspath",m_savedir.path());
    qapp.setProperty("pluginspath",QLatin1String (ROOM_SYSTEM_SERVERPLUGINS));

#ifndef _WIN32
    const QLatin1String servicename = QLatin1String(ROOM_SERVICENAME);
    QDBusConnection dbusconnection = QDBusConnection::connectToBus(QDBusConnection::SessionBus, servicename);
    if ( !dbusconnection.isConnected() )
    {
        qCritical() << "DBus Error:" << dbusconnection.lastError().name() << dbusconnection.lastError().message();
        return 0;
    } else if (!dbusconnection.registerService(servicename) )
    {
        qCritical() << "Another instance of this program is already running." << dbusconnection.lastError().name() << dbusconnection.lastError().message();
        return 0;
    }
#endif

    qInstallMsgHandler(myMessageOutput);

    qDebug() << QCoreApplication::applicationName().toAscii().data() << ":" << QCoreApplication::applicationVersion().toAscii().data();

    network = new NetworkController();
    ServiceController* services = new ServiceController();
    network->setServiceController(services);
    network->connect(services,SIGNAL(serviceSync(AbstractServiceProvider*)),SLOT(serviceSync(AbstractServiceProvider*)));
    network->connect(services,SIGNAL(statetrackerSync(AbstractStateTracker*)),SLOT(statetrackerSync(AbstractStateTracker*)));
    network->start();

    int r = 0;
    if (argc<=1 || strcmp("--shutdown", argv[1])!=0)
        r = qapp.exec();

	#ifndef _WIN32
		dbusconnection->unregisterService(servicename);
	#endif

    qDebug() << "Shutdown: Network server";
    delete network;
    network = 0;

    qDebug() << "Shutdown: Service Controller";
    delete services;
    services = 0;

    // restart program
    if (r == EXIT_WITH_RESTART) {
        r = 0;
        if (argc>1 && strcmp("--norestart", argv[1])==0) {
            qDebug() << "Shutdown: Start another instance not allowed";
        } else {
            qDebug() << "Shutdown: Start another instance";
            QProcess::startDetached(QString::fromAscii(argv[0]));
        }
    }

    return r;
}
