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

	Purpose: https server
*/

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
    m_state = Invalid;
}

ManagedDeviceList::~ManagedDeviceList() {
    m_udevMonitorNotifier->setEnabled(false);
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
    m_udevMonitorNotifier->setEnabled(false);
    struct udev_device *dev;
    if (!(dev = udev_monitor_receive_device(udev_mon))) {
        qDebug() <<"Failed to get udev device object from monitor.";
        return;
    }

    const char* devpath = udev_device_get_devpath(dev);
    if (strncmp(devpath,"/devices",8)!=0) return;

    processDevice(dev);
    udev_device_unref(dev);
    m_udevMonitorNotifier->setEnabled(true);
}

void ManagedDeviceList::processDevice(struct udev_device *dev)
{
    Q_ASSERT(dev);
    // Ignore child devices
    //if (udev_device_get_property_value(dev, "ID_VENDOR_ID") == 0) return;

    const char* attr = udev_device_get_property_value(dev, "RCTR_DEVICE");
    if (!attr) {
        return;
    }

    struct udev_device * usbdev = udev_device_get_parent_with_subsystem_devtype(dev,"usb","usb_device");
    if (usbdev == 0)
        return;

    const char* action = udev_device_get_action(dev);
    const char* sys_path_t = udev_device_get_property_value(dev, "DEVPATH");
    if (!sys_path_t) return;
    const char* dev_path_t = udev_device_get_devnode(dev);
    if (!dev_path_t) return;
    const QByteArray sys_path = sys_path_t;
    const QByteArray dev_path = dev_path_t;

    if (action && strcmp(action,"remove")==0) {
        /* get device */
        ManagedDevice* device = m_devices.value(sys_path);
        if (!device) return;
        emit deviceRemoved(device);
        m_devices.remove(sys_path);
    } else { // add device
        ManagedDevice* device = m_devices.value(sys_path);
        // if device already present, do nothing */
        if (device) return;
        // create new device object with settings
        device = new ManagedDevice();
        device->devPath = dev_path;
        device->sysPath = sys_path;

        device->info = QByteArray(udev_device_get_sysattr_value(usbdev,"product")) +
                       " - " + QByteArray(udev_device_get_sysattr_value(usbdev,"manufacturer")) +
                       " (" + dev_path + ")";
        device->udid = QByteArray(udev_device_get_sysattr_value(usbdev, "idVendor"))  +
                       QByteArray(udev_device_get_sysattr_value(usbdev, "idProduct")) +
                       QByteArray(udev_device_get_sysattr_value(usbdev, "serial"));
        if (device->udid.isEmpty()) device->udid = dev_path;

        /* add to list */
        m_devices.insert(sys_path, device);

        emit deviceAdded(device);
    }
}
void ManagedDeviceList::start() {

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
    } else if (udev_monitor_filter_add_match_subsystem_devtype (udev_mon, "input", NULL) != 0) {
        qWarning() << "UDev monitor filter failed:" << errno;
        udev_monitor_unref (udev_mon);
        // Enable receiving of udev events
//     } else if (udev_monitor_filter_add_match_subsystem_devtype (udev_mon, "usb", "usb_device") != 0) {
//         qWarning() << "UDev monitor filter failed:" << errno;
//         udev_monitor_unref (udev_mon);
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
//     if (udev_enumerate_add_match_sysattr  (devenum, "RCTR_DEVICE", "1"))
//     {
//         udev_enumerate_unref (devenum);
//         m_state = Invalid;
//         return;
//     }
    if (udev_enumerate_add_match_subsystem (devenum, "input"))
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
