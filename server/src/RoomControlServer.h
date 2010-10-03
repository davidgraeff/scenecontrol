#ifndef RoomControlServer_H
#define RoomControlServer_H

#include <QtCore/QObject>
#include <QDBusConnection>

class QTimerEvent;
class ProjectorController;
class ProgramStateTracker;
class RemoteController;
class CinemaController;
class NetworkController;
class IOController;
class LedController;
class CurtainController;
class MediaController;
class EventController;
class Factory;
class AbstractStateTracker;

/**
 * Used as singleton to access all other controllers.
 * Initiate controllers and frees them again.
 */
class RoomControlServer: public QObject
{
    Q_OBJECT
public:
    virtual ~RoomControlServer();
    static RoomControlServer* createInstance ();
    static RoomControlServer* getRoomControlServer();
    static LedController* getLedController();
    static IOController* getIOController();
    static CurtainController* getCurtainController();
    static EventController* getEventController();
    static MediaController* getMediaController();
    static CinemaController* getCinemaController();
    static NetworkController* getNetworkController();
    static RemoteController* getRemoteController();
    static ProjectorController* getProjectorController();
    static Factory* getFactory();
    static QDBusConnection* getBus();
    QList<AbstractStateTracker*> getStateTracker();
    ProgramStateTracker* programStateTracker;
private:
    static RoomControlServer* instance;
    RoomControlServer ( QDBusConnection dbusconnection );
    RoomControlServer ();
    bool init();
    QDBusConnection dbusconnection;
    LedController* ledController;
    IOController* ioController;
    CurtainController* curtainController;
    EventController* eventController;
    MediaController* mediaController;
    CinemaController* cinemaController;
    NetworkController* networkController;
    RemoteController* remoteController;
    ProjectorController* projectorController;
    Factory* factory;
private Q_SLOTS:
    void dumpStats();
    void dumpLedCount();
    void dumpIOCount();
protected:
    void timerEvent(QTimerEvent *event);
};

#endif // RoomControlServer_H
