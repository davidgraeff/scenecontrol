#ifndef LIRI_DAEMON_CONNECTION_H_
#define LIRI_DAEMON_CONNECTION_H_

#include <QString>
#include <QObject>
#include <QList>
#include <QMap>
#include <QStringList>
#include <abstractserviceprovider.h>
#include <events/eventremotekey.h>

class RemoteControlKeyStateTracker;
class RemoteControlStateTracker;
class AbstractStateTracker;
class OrgLiriControlInterface;
class OrgLiriDeviceInterface;

#define LIRI_DBUS_SERVICE "org.liri.Devices"
#define LIRI_DBUS_OBJECT_RECEIVERS "/org/liri/Devicelist"

class RemoteController: public QObject {
    Q_OBJECT
public:
    RemoteController(QObject* parent = 0);
    ~RemoteController();
    QString mode() const;
    void setMode(const QString& mode);
    QList<AbstractStateTracker*> getStateTracker();
    int countReceiver() const;
    bool isConnected() const;
    void registerKeyEvent(EventRemoteKey* event);
    bool isRegistered(EventRemoteKey* event) ;
	void setRepeatingEvent(EventRemoteKey* event){m_repeating=event;}

private:
    QStringList parseIntrospect();
    OrgLiriControlInterface* m_control;
    QMap< QString, OrgLiriDeviceInterface* > m_devices;
    QString m_mode;
    QMultiMap<QString, EventRemoteKey*> m_keyevents;
    RemoteControlStateTracker* m_statetracker;
    RemoteControlKeyStateTracker* m_statetrackerKey;
	EventRemoteKey* m_repeating;

private Q_SLOTS:
    void slotServiceUnregistered(const QString& service);
    void slotServiceRegistered(const QString& service);
    void deviceAdded(const QString& uid);
    void deviceRemoved(const QString& uid);
    void keySlot(const QString &keycode, const QString &keyname, uint channel, int pressed);
    void keyEventDestroyed ( QObject * obj);
Q_SIGNALS:
//     void key(const QString &keycode, const QString &keyname, uint channel, int pressed);
//    void changedMode(const QString &mode);
};

#endif
