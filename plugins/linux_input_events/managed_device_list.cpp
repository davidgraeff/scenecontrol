/****************************************************************************
** This file is part of the linux remotes project
**
** Use this file under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <QDebug>
#include <QCoreApplication>
extern "C"
{
#include <libudev.h>
#include <poll.h>
#include <errno.h>
}

#include "managed_device_list.h"

ManagedDeviceList::ManagedDeviceList() {
    // connect to udev and set up monitor for new/removed receivers
    udev_mon = 0;
    udev = 0;
    udev_monitor_fd = 0;
    m_udevMonitorNotifier = 0;

    udev = udev_new ();
    if (udev == 0) {
        m_state = Invalid;
        qWarning() << "UDev failed:" << errno;
        return;
    }

    udev_mon = udev_monitor_new_from_netlink (udev, "udev");
    if (udev_mon == 0)
    {
        m_state = ValidWithoutMonitoring;
        qWarning() << "UDev monitor failed:" << errno;
        // Add filter to only receive events for the usb subsystem
    } else if (udev_monitor_filter_add_match_subsystem_devtype (udev_mon, "hidraw", NULL) != 0) {
        qWarning() << "UDev monitor filter failed:" << errno;
        udev_monitor_unref (udev_mon);
        // Enable receiving of udev events
    } else if (udev_monitor_enable_receiving (udev_mon) < 0) {
        qWarning() << "UDev monitor enabling failed:" << strerror(errno);
        udev_monitor_unref (udev_mon);
        udev_mon = 0;
        // Get file descriptor
    } else if ((udev_monitor_fd = udev_monitor_get_fd(udev_mon)) < 0) {
        qWarning() << "Failed to get udev monitor fd.";
        udev_monitor_unref (udev_mon);
        udev_mon = 0;
        // Watch file descriptor
    } else {
        m_udevMonitorNotifier = new QSocketNotifier(udev_monitor_fd, QSocketNotifier::Read);
        connect(m_udevMonitorNotifier, SIGNAL(activated(int)), SLOT(udevActivity(int)));
        m_udevMonitorNotifier->setEnabled(true);
    }

    // enumerate all existing devices and add receivers
    struct udev_enumerate *devenum = udev_enumerate_new (udev);
    if (devenum == 0) {
        m_state = Invalid;
        return;
    }
    if (udev_enumerate_add_match_subsystem (devenum, "hidraw"))
    {
        udev_enumerate_unref (devenum);
        m_state = Invalid;
        return;
    }

udev_enumerate_scan_devices (devenum);
    struct udev_list_entry *devlist = udev_enumerate_get_list_entry (devenum);
    struct udev_list_entry *deventry;
    udev_list_entry_foreach (deventry, devlist)
    {
        const char *path = udev_list_entry_get_name (deventry);
        struct udev_device *dev = udev_device_new_from_syspath (udev, path);
        processDevice(dev);
        udev_device_unref (dev);
    }
    udev_enumerate_unref (devenum);
    m_state = Valid;
}

ManagedDeviceList::~ManagedDeviceList() {
    delete m_udevMonitorNotifier;
    close (udev_monitor_fd);

    if (udev_mon)
        udev_monitor_unref(udev_mon);

    if (udev)
        udev_unref(udev);

    udev = 0;
    udev_mon = 0;

    qDeleteAll(m_devices);
}

ManagedDeviceList::StateEnum ManagedDeviceList::getState()
{
    return m_state;
}

void ManagedDeviceList::udevActivity ( int socket )
{
    Q_UNUSED(socket);
    struct udev_device *dev;
    if (!(dev = udev_monitor_receive_device(udev_mon))) {
        qDebug() <<"Failed to get udev device object from monitor.";
        return;
    }

    const char* devpath = udev_device_get_devpath(dev);
    if (strncmp(devpath,"/devices",8)!=0) return;

    processDevice(dev);
    udev_device_unref(dev);
}

void ManagedDeviceList::processDevice(struct udev_device *dev)
{
    Q_ASSERT(dev);
    // Ignore child devices
    //if (udev_device_get_property_value(dev, "ID_VENDOR_ID") == 0) return;

    // Ignore devices without special ability
    //const char* driver = udev_device_get_property_value(dev, "special");
    //if (!driver) return;

    const char* action = udev_device_get_action(dev);
    const char* uid_t = udev_device_get_property_value(dev, "DEVPATH");
    if (!uid_t) return;
    const char* dev_path_t = udev_device_get_devnode(dev);
    if (!dev_path_t) return;
    const QString uid = QString::fromAscii(uid_t);
    const QString dev_path = QString::fromAscii(dev_path_t);

    if (action && strcmp(action,"remove")==0) {
        /* get device */
        ManagedDevice* device = m_devices.value(uid);
        if (!device) return;
        emit deviceRemoved(device);
        m_devices.remove(uid);
    } else { // add device
        ManagedDevice* device = m_devices.value(uid);
        // if device already present, do nothing */
        if (device) return;
        // create new device object with settings
        device = new ManagedDevice();
        device->devPath = dev_path;
        device->sysPath = uid;

        struct udev_device * usbdev = udev_device_get_parent_with_subsystem_devtype(dev,"usb","usb_device");
        if (usbdev) {
            device->info = QString::fromUtf8(udev_device_get_sysattr_value(usbdev,"product")) +
                           QLatin1Literal(" (") +  QString::fromUtf8(udev_device_get_sysattr_value(usbdev,"manufacturer")) + QLatin1Literal(")");
        }

        /* add to list */
        m_devices.insert(uid, device);

        emit deviceAdded(device);
    }
}
