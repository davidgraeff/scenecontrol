#pragma once

#include <QString>
#include <QObject>
#include <QMap>
#include <QSocketNotifier>

class DeviceInstance;
struct udev_monitor;
struct udev;
struct udev_device;

class ManagedDevice {
public:
	QString devPath;
	QString sysPath;
	QString info;
};

class ManagedDeviceList : public QObject
{
    Q_OBJECT
public:
	enum StateEnum {
		Invalid,
		Valid,
		ValidWithoutMonitoring
	};
    ManagedDeviceList();
    ~ManagedDeviceList();
    ManagedDeviceList::StateEnum getState();
    void processDevice(struct udev_device *dev);
Q_SIGNALS:
    void deviceAdded(ManagedDevice*);
    void deviceRemoved(ManagedDevice*);
private Q_SLOTS:
    void udevActivity ( int socket );
private:
    /* All device instances. Accessible through their unique sys path */
    QMap<QString, ManagedDevice*> m_devices;
    QSocketNotifier* m_udevMonitorNotifier;
    StateEnum m_state;
    struct udev_monitor *udev_mon;
    struct udev *udev;
    int udev_monitor_fd;
};
