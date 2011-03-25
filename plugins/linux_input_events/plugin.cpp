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

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

int plugin::m_repeat = 0;
int plugin::m_repeatInit = 0;

plugin::plugin() {
    m_devicelist = new ManagedDeviceList();
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
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if (name == QLatin1String("repeat")) m_repeat = value.toInt();
    else if (name == QLatin1String("repeat_init")) m_repeatInit = value.toInt();
}

void plugin::execute ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

bool plugin::condition ( const QVariantMap& data )  {
    Q_UNUSED ( data );
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    if ( IS_ID ( "inputevent" ) ) {
        // entfernen
        const QString uid = UNIQUEID();

        QMutableMapIterator<QString, InputDevice* > it ( m_devices );
        while ( it.hasNext() ) {
            it.value()->unregisterKey(uid);
            if ( it.value()->isClosable() )
                it.remove();
        }

        InputDevice* inputdevice = m_devices.value(DATA ( "inputdevice") );
        if (inputdevice) inputdevice->registerKey(uid, DATA ( "kernelkeyname" ), BOOLDATA("repeat"));
    }
}

void plugin::otherPropertyChanged ( const QVariantMap& data, const QString& sessionid ) {
    if ( IS_ID ( "selected_input_device" ) && m_sessions.contains ( sessionid ) ) {
        InputDevice* inputdevice = m_devices.value(DATA ( "inputdevice") );
        if (!inputdevice) return;
        inputdevice->connectSession(sessionid);
    }
}

void plugin::session_change ( const QString& id, bool running ) {
    PluginSessionsHelper::session_change ( id, running );
    if ( running ) return;
    foreach (InputDevice* device, m_devices) {
        device->disconnectSession(id);
    }
}

QMap<QString, QVariantMap> plugin::properties(const QString& sessionid) {
Q_UNUSED(sessionid);
    QMap<QString, QVariantMap> l;
    return l;
}

void plugin::deviceAdded(ManagedDevice* device) {
    PROPERTY ( "inputdevice" );
    data[QLatin1String ( "device_path" ) ] = device->devPath;
    data[QLatin1String ( "device_info" ) ] = device->info;
    m_server->property_changed ( data );
    InputDevice* inputdevice = m_devices.value(device->devPath);
    if (!inputdevice) {
        inputdevice = new InputDevice(this);
        m_devices[device->devPath] = inputdevice;
    }
    inputdevice->setDevice(device);
}

void plugin::deviceRemoved(ManagedDevice* device) {
    PROPERTY ( "inputdevice" );
    data[QLatin1String ( "device_path" ) ] = device->devPath;
    m_server->property_changed ( data );
    InputDevice* inputdevice = m_devices.value(device->devPath);
    if (inputdevice && inputdevice->isClosable()) {
        delete m_devices.take(device->devPath);
    }
}





InputDevice::InputDevice(plugin* plugin) : m_plugin(plugin) {
    connect(&m_repeattimer,SIGNAL(timeout()),SLOT(repeattrigger()));
    m_repeattimer.setSingleShot(true);
    connect(&m_file,SIGNAL(readyRead()),SLOT(eventData()));
}
InputDevice::~InputDevice() {
    m_file.close();
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
    if (isClosable()) m_file.close();
}
void InputDevice::setDevice(ManagedDevice* device) {
    m_device = device;
    if (!device)
        m_file.close();
    else if (!m_file.isOpen() && !isClosable()) {
		m_file.setFileName(device->devPath);
        m_file.open(QIODevice::ReadOnly);
    }
}

void InputDevice::unregisterKey(QString uid) {
    QMutableMapIterator<QString, EventKey* > it ( m_keyToUids );
    while ( it.hasNext() ) {
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
    while ((uint)m_file.bytesAvailable()>=sizeof(struct input_event)) {
        struct input_event ev;
        if (m_file.read((char*)&ev,sizeof(struct input_event)) == -1) continue;
        if (EV_KEY != ev.type) continue;
        if ((ev.value == KEY_PRESS) || (ev.value == KEY_KEEPING_PRESSED)) {
            const QString& kernelkeyname = m_plugin->m_keymapping.value(ev.code);
            // properties
            {
                // last key property. Will be propagated to interested clients only.
                PROPERTY ( "lastinputkey" );
                data[QLatin1String ( "kernelkeyname" ) ] = kernelkeyname;
                data[QLatin1String ( "inputdevice" ) ] = m_device->devPath;

                // Propagate to all interested clients
                foreach(QString sessionid, m_sessionids) {
                    m_plugin->m_server->property_changed(data, sessionid);
                }
            }
            // repeat stop
            m_repeattimer.stop();
            m_lastkey = kernelkeyname;
            repeattrigger(true);
        } else {
            m_repeattimer.stop();
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

    if (!event->repeat) return;

    if (initial_event)
        m_repeattimer.start(initial_event?m_plugin->m_repeatInit:m_plugin->m_repeat);
}
