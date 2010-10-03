#include "RoomControlServer.h"

#include <QDebug>
#include "networkcontroller.h"
#include "media/eventcontroller.h"
#include "media/mediacontroller.h"
#include "media/cinemacontroller.h"
#include <QCoreApplication>
#include "factory.h"
#include "remotecontrol/remotecontroller.h"
#include "stateTracker/programstatetracker.h"
#include <qprocess.h>
#include <QDBusError>
#include "media/cinemacontroller.h"
#include "actors/abstractactor.h"
#include "conditions/abstractcondition.h"
#include "profile/serviceproviderprofile.h"
#include "media/playlist.h"
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include "projectorcontroller.h"
#include "ledcontroller.h"
#include "iocontroller.h"
#include "curtaincontroller.h"

RoomControlServer* RoomControlServer::instance = 0;
const QLatin1String servicename = QLatin1String("org.roomcontrol");

void myMessageOutput(QtMsgType type, const char *msg)
{
    if (RoomControlServer::getRoomControlServer() && RoomControlServer::getNetworkController())
        RoomControlServer::getNetworkController()->log(msg);
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        abort();
    }
}

/* first, here is the signal handler */
static void catch_int(int )
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
    QCoreApplication::exit(0);
}

RoomControlServer::RoomControlServer (QDBusConnection dbusconnection)
        : programStateTracker(0), dbusconnection(dbusconnection), ledController(0),
        ioController(0),curtainController(0),
        eventController(0), mediaController(0), cinemaController(0), networkController(0),
        remoteController(0), projectorController(0), factory(0)
{
}

RoomControlServer::~RoomControlServer()
{
    qDebug() << "Shutdown: Network server";
    delete networkController;
    networkController = 0; // Avoid trying to send debug messages to the network controller
    qDebug() << "Shutdown: Media controller";
    delete mediaController;
    qDebug() << "Shutdown: MPRIS controller";
    delete cinemaController;
    qDebug() << "Shutdown: Event controller";
    delete eventController;
    qDebug() << "Shutdown: RemoteControl controller";
    delete remoteController;
    qDebug() << "Shutdown: Projector controller";
    delete projectorController;
    qDebug() << "Shutdown: Led connection";
    delete ledController;
    qDebug() << "Shutdown: IO connection";
    delete ioController;
    qDebug() << "Shutdown: Curtain connection";
    delete curtainController;
    qDebug() << "Shutdown: Factory";
    delete factory;
    qDebug() << "Shutdown: Controller";
    delete programStateTracker;
    qDebug() << "Shutdown: IPC connection";
    dbusconnection.unregisterService(servicename);
    instance = 0;
}

bool RoomControlServer::init()
{

    // Network Controller must be initialized first
    qDebug() << "Start: Network controller";
    networkController = new NetworkController();
    // Init all other controllers
    qDebug() << "Start: Event controller";
    eventController = new EventController();
    qDebug() << "Start: Mpris controller";
    cinemaController = new CinemaController();
    qDebug() << "Start: Media controller";
    mediaController = new MediaController();
    qDebug() << "Start: RemoteControl controller";
    remoteController = new RemoteController();
    qDebug() << "Start: Projector controller";
    projectorController = new ProjectorController();
    qDebug() << "Start: Led controller";
    ledController = new LedController();
    qDebug() << "Start: IO controller";
    ioController = new IOController();
    qDebug() << "Start: Curtain controller";
    curtainController = new CurtainController();
    // Factory must be initialized last
    qDebug() << "Start: Factory";
    factory = new Factory();
    factory->load();
    qDebug() << "Start: Others";
    // some other initalisations
    programStateTracker = new ProgramStateTracker();
    ledController->connectTo ( QHostAddress ( QLatin1String("192.168.1.8") ), 2702 );
    curtainController->connectTo ( QHostAddress ( QLatin1String("192.168.1.8") ), 2704 );
    connect ( ledController,SIGNAL ( dataAvailable() ),SLOT ( dumpLedCount() ) );
    connect ( ioController,SIGNAL ( dataAvailable() ),SLOT ( dumpIOCount() ) );
	
    mediaController->activateFavourite();
	startTimer(500);
    return networkController->start();
}

QList<AbstractStateTracker*> RoomControlServer::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    l.append(programStateTracker);
    l.append(eventController->getStateTracker());
    l.append(cinemaController->getStateTracker());
    l.append(mediaController->getStateTracker());
    l.append(ledController->getStateTracker());
    l.append(ioController->getStateTracker());
    l.append(curtainController->getStateTracker());
    l.append(remoteController->getStateTracker());
    l.append(projectorController->getStateTracker());
    return l;
}

void RoomControlServer::dumpStats()
{
    int e= 0, c= 0, a= 0, p = 0, pl = 0, s = getStateTracker().count();
    for (int i=0;i< factory->m_providerList.size();++i) {
        AbstractServiceProvider* pr= factory->m_providerList[i];
        if (qobject_cast<AbstractActor*>(pr)) ++a;
        else if (qobject_cast<AbstractCondition*>(pr)) ++c;
        else if (qobject_cast<AbstractEvent*>(pr)) ++e;
        else if (qobject_cast<ProfileCollection*>(pr)) ++p;
        else if (qobject_cast<Playlist*>(pr)) ++pl;
    }
    qDebug() << "Profiles    :" << p;
    qDebug() << "Events      :" << e;
    qDebug() << "Conditions  :" << c;
    qDebug() << "Actions     :" << a;
    qDebug() << "Playlists   :" << pl;
    qDebug() << "StateTracker:" << s;
    qDebug() << "------------";
    qDebug() << "All         :" << pl+a+c+e+p+s;
}

void RoomControlServer::dumpIOCount() {
    disconnect ( ioController,SIGNAL ( dataAvailable() ),this,SLOT ( dumpIOCount() ) );
    qDebug() << "Pins        :" << ioController->countPins();

}
void RoomControlServer::dumpLedCount() {
    disconnect ( ledController,SIGNAL ( dataAvailable() ),this,SLOT ( dumpLedCount() ) );
    qDebug() << "Channels    :" << ledController->countChannels();

}

RoomControlServer* RoomControlServer::createInstance ()
{
    if ( instance == 0 )
    {
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

        qDebug() << QCoreApplication::applicationName().toAscii().data() <<":"
        << QCoreApplication::applicationVersion().toAscii().data()
        << __TIMESTAMP__;

        instance = new RoomControlServer (dbusconnection);
        if (!instance->init()) {
            delete instance;
            instance = 0;
        }
    }
    return instance;
}

RoomControlServer* RoomControlServer::getRoomControlServer ()
{
    return instance;
}

LedController* RoomControlServer::getLedController()
{
    Q_ASSERT ( instance );
    return instance->ledController;
}

IOController* RoomControlServer::getIOController()
{
    Q_ASSERT ( instance );
    return instance->ioController;
}

CurtainController* RoomControlServer::getCurtainController()
{
    Q_ASSERT ( instance );
    return instance->curtainController;
}

EventController* RoomControlServer::getEventController()
{
    Q_ASSERT ( instance );
    return instance->eventController;
}

MediaController* RoomControlServer::getMediaController()
{
    Q_ASSERT ( instance );
    return instance->mediaController;
}

NetworkController* RoomControlServer::getNetworkController()
{
    Q_ASSERT ( instance );
    return instance->networkController;
}

RemoteController* RoomControlServer::getRemoteController()
{
    Q_ASSERT ( instance );
    return instance->remoteController;
}

Factory* RoomControlServer::getFactory()
{
    Q_ASSERT ( instance );
    return instance->factory;
}

CinemaController* RoomControlServer::getCinemaController()
{
    Q_ASSERT ( instance );
    return instance->cinemaController;
}

ProjectorController* RoomControlServer::getProjectorController() {
    Q_ASSERT ( instance );
    return instance->projectorController;
}

QDBusConnection* RoomControlServer::getBus()
{
    Q_ASSERT ( instance );
    return &(instance->dbusconnection);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);

    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String("RoomControlServer"));
    qapp.setApplicationVersion(QLatin1String("1.8 "__DATE__ "-" __TIME__));
    qapp.setOrganizationName(QLatin1String("davidgraeff"));

    qInstallMsgHandler(myMessageOutput);
    RoomControlServer* i = RoomControlServer::createInstance();

    int r = -1;

    if (i)
        r = qapp.exec();
    delete i;
    i = 0;

    // restart program
    if (r == 1) {
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
void RoomControlServer::timerEvent(QTimerEvent* event) {
	killTimer(event->timerId());
    dumpStats();
}
