/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QDebug>
#include <QtPlugin>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>
#include <sys/ioctl.h>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2

#include "parse.h"

#include "plugin.h"
#include "configplugin.h"
#include "managed_device_list.h"
#include <qfileinfo.h>

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

int plugin::m_repeat = 0;
int plugin::m_repeatInit = 0;

plugin::plugin() {
    m_devicelist = new ManagedDeviceList();
    connect(m_devicelist,SIGNAL(deviceAdded(ManagedDevice*)),SLOT(deviceAdded(ManagedDevice*)));
    connect(m_devicelist,SIGNAL(deviceRemoved(ManagedDevice*)),SLOT(deviceRemoved(ManagedDevice*)));
    _config ( this );
}

plugin::~plugin() {
    delete m_devicelist;
}

void plugin::clear() {}
void plugin::initialize() {
    m_server->register_listener ( QLatin1String ( "selected_input_device" ) );
    for (int i=0;keynames[i].name;++i) {
        m_keymapping[keynames[i].value] = QString::fromAscii(keynames[i].name);
    }
    m_devicelist->start();
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if (name == QLatin1String("repeat")) m_repeat = value.toInt();
    else if (name == QLatin1String("repeat_init")) m_repeatInit = value.toInt();
}

void plugin::execute ( const QVariantMap& data, const QString& sessionid ) {
    if ( ServiceID::isId(data, "selected_input_device" ) && m_sessions.contains ( sessionid ) ) {
		QMap<QString, InputDevice*>::iterator it = m_devices.begin();
		for (;it != m_devices.end();++it) {
			(*it)->disconnectSession(sessionid);
		}
        InputDevice* inputdevice = m_devices.value(DATA ( "udid") );
        if (!inputdevice) return;
        inputdevice->connectSession(sessionid);
    }
}

bool plugin::condition ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED(sessionid);
    Q_UNUSED ( data );
    return false;
}

void plugin::event_changed ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED(sessionid);
    if ( ServiceID::isId(data, "inputevent" ) ) {
        // entfernen
        const QString uid = ServiceType::uniqueID(data);

        QMutableMapIterator<QString, InputDevice* > it ( m_devices );
        while ( it.hasNext() ) {
            it.next();
            it.value()->unregisterKey(uid);
            if ( it.value()->isClosable() )
                it.remove();
        }

        InputDevice* inputdevice = m_devices.value(DATA ( "udid") );
        if (inputdevice) inputdevice->registerKey(uid, DATA ( "kernelkeyname" ), BOOLDATA("repeat"));
    }
}

void plugin::session_change ( const QString& id, bool running ) {
    PluginSessionsHelper::session_change ( id, running );
    if ( running ) return;
    foreach (InputDevice* device, m_devices) {
        device->disconnectSession(id);
    }
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "inputdevice", "udid").getData());
    foreach(InputDevice* device, m_devices) {
        l.append(createServiceOfDevice(device->device()).getData());
    }
    return l;
}

void plugin::deviceAdded(ManagedDevice* device) {
    m_server->property_changed ( createServiceOfDevice(device).getData() );
    InputDevice* inputdevice = m_devices.value(device->udid);
    if (!inputdevice) {
        inputdevice = new InputDevice(this);
        m_devices[device->udid] = inputdevice;
    }
    inputdevice->setDevice(device);
}

void plugin::deviceRemoved(ManagedDevice* device) {
    ServiceCreation sc = ServiceCreation::createModelRemoveItem(PLUGIN_ID, "inputdevice" );
    sc.setData("udid", device->udid);
    m_server->property_changed ( sc.getData() );
    delete m_devices.take(device->udid);
}

ServiceCreation plugin::createServiceOfDevice(ManagedDevice* device) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "inputdevice" );
    sc.setData("path", device->devPath);
    sc.setData("info", device->info);
    sc.setData("udid", device->udid);
    return sc;
}

InputDevice::InputDevice(plugin* plugin) : m_plugin(plugin), m_socketnotifier(0), fd(0), m_device(0) {
    connect(&m_repeattimer,SIGNAL(timeout()),SLOT(repeattrigger()));
    m_repeattimer.setSingleShot(true);
}

InputDevice::~InputDevice() {
    disconnectDevice();
}

ManagedDevice* InputDevice::device() {
    return m_device;
    delete m_socketnotifier;
}

bool InputDevice::isClosable() {
    return (m_sessionids.isEmpty() && m_keyToUids.isEmpty());
}

void InputDevice::connectSession(const QString& sessionid) {
    m_sessionids.insert(sessionid);
    setDevice(m_device);
}
void InputDevice::disconnectSession(const QString& sessionid) {
    m_sessionids.remove(sessionid);
    if (m_sessionids.isEmpty()) {
        disconnectDevice();
    }
}

void InputDevice::disconnectDevice() {
    if (fd) close(fd);
    fd = 0;
    delete m_socketnotifier;
    m_socketnotifier = 0;
}

void InputDevice::setDevice(ManagedDevice* device) {
    if (device==m_device && fd) {
        qDebug() << "setdevice no open";
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "input.device.selected" );
        sc.setData("udid", m_device->udid);
        sc.setData("listen", true);
        sc.setData("errormsg", QString());
        // Propagate to all interested clients
        foreach(QString sessionid, m_sessionids) {
            m_plugin->m_server->property_changed(sc.getData(), sessionid);
        }
        return;
    }

    m_device = device;
    disconnectDevice();
    if (device) {
        // only reconnect to new device if a client is actually listening or events are registered
        if (m_sessionids.isEmpty() && m_keyToUids.isEmpty()) return;

        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "input.device.selected" );
        sc.setData("udid", m_device->udid);

        if (!QFileInfo(m_device->devPath).isReadable()) {
            // error
            sc.setData("listen", false);
            sc.setData("errormsg", QString(QLatin1String("InputDevice ") + m_device->devPath + QLatin1String(" open failed. No access rights!")));
        } else {
            fd = open(m_device->devPath.toUtf8(), O_RDONLY|O_NDELAY);
            if (fd!=-1) {
                m_socketnotifier = new QSocketNotifier(fd, QSocketNotifier::Read);
                connect(m_socketnotifier, SIGNAL(activated(int)), this, SLOT(eventData()));
                // success
                sc.setData("listen", true);
                sc.setData("errormsg", QString());
            } else {
                sc.setData("listen", false);
                sc.setData("errormsg", QString(QLatin1String("InputDevice open failed: ") + m_device->devPath));
            }
        }
        // Propagate to all interested clients
        foreach(QString sessionid, m_sessionids) {
            m_plugin->m_server->property_changed(sc.getData(), sessionid);
        }
    }
}

void InputDevice::unregisterKey(QString uid) {
    QMutableMapIterator<QString, EventKey* > it ( m_keyToUids );
    while ( it.hasNext() ) {
        it.next();
        it.value()->uids.remove(uid);
        if ( it.value()->uids.isEmpty() )
            it.remove();
    }
}
void InputDevice::registerKey(QString uid, QString key, bool repeat) {
    m_keyToUids[key]->uids.insert(uid);
    m_keyToUids[key]->repeat = repeat;
}
void InputDevice::eventData() {
    static char readbuff[sizeof(struct input_event)] = {0};
    static struct input_event* ev = (struct input_event*)readbuff;
	static unsigned int readbuffOffset = 0;
	
    while ( 1 ) {
		int ret = read(fd, readbuff+readbuffOffset, sizeof(struct input_event)-readbuffOffset);
		if (ret == -1) return;
		readbuffOffset += ret;
		if (readbuffOffset < sizeof(struct input_event)) return;
		readbuffOffset = 0;
        
        if (ev->type != EV_KEY) return;
        m_repeattimer.stop();
        if ((ev->value == KEY_PRESS) || (ev->value == KEY_KEEPING_PRESSED)) {
            const QString& kernelkeyname = m_plugin->m_keymapping.value(ev->code);
            qDebug() << "key event" << m_device->devPath << kernelkeyname;
            // properties
            {
                // last key property. Will be propagated to interested clients only.
                ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "input.device.key" );
                sc.setData("kernelkeyname", kernelkeyname);
                sc.setData("udid", m_device->udid);

                // Propagate to all interested clients
                foreach(QString sessionid, m_sessionids) {
                    m_plugin->m_server->property_changed(sc.getData(), sessionid);
                }
            }
            m_lastkey = kernelkeyname;
            repeattrigger(true);
        }
    }
}
void InputDevice::repeattrigger(bool initial_event) {
    QMap<QString, EventKey*>::iterator it = m_keyToUids.find(m_lastkey);
    if (it == m_keyToUids.end()) return;

    const EventKey* event = *it;
    foreach (QString uid, event->uids) {
        m_plugin->m_server->event_triggered(uid);
    }

    if (event->repeat)
        m_repeattimer.start(initial_event?m_plugin->m_repeatInit:m_plugin->m_repeat);
}

