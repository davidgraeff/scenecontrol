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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <errno.h>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2

#include "parse.h"

#include "plugin.h"
#include "managed_device_list.h"
#include <qfileinfo.h>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& instanceid) : AbstractPlugin(instanceid) {
	m_dontgrab = 0;
    m_repeat = 0;
    m_repeatInit = 0;
    m_devicelist = new ManagedDeviceList();
    connect ( m_devicelist,SIGNAL ( deviceAdded ( ManagedDevice* ) ),SLOT ( deviceAdded ( ManagedDevice* ) ) );
    connect ( m_devicelist,SIGNAL ( deviceRemoved ( ManagedDevice* ) ),SLOT ( deviceRemoved ( ManagedDevice* ) ) );
}

plugin::~plugin() {
    clear();
    delete m_devicelist;
}

void plugin::clear() {
    m_keymapping.clear();
    qDeleteAll(m_devices);
    m_devices.clear();
	m_events.clear();
}

void plugin::initialize() {
    clear();
    for ( int i=0;keynames[i].name;++i ) {
        m_keymapping[keynames[i].value] = keynames[i].name;
    }
    m_devicelist->start();
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("repeat")))
        m_repeat = data[QLatin1String("repeat")].toInt();
    if (data.contains(QLatin1String("repeat_init")))
        m_repeatInit = data[QLatin1String("repeat_init")].toInt();
    if (data.contains(QLatin1String("dontgrab")))
        m_dontgrab = data[QLatin1String("dontgrab")].toBool();
}

void plugin::inputevent ( const QString& _id, const QString& collection_, const QString& inputdevice, const QString& kernelkeyname, bool repeat) {
	QByteArray __id = _id.toAscii();
    // Add to input events list
    EventInputStructure s;
    s.collectionuid = collection_.toAscii();
    s.inputdevice = inputdevice.toAscii();
    s.kernelkeyname = kernelkeyname.toAscii();
    s.repeat = repeat;
    m_events.insert(__id, s);

    // If device for this event already exists, register key
    InputDevice* inputdeviceObj = m_devices.value ( inputdevice.toAscii() );
    if ( !inputdeviceObj )
        return;
    m_devices_by_eventsids.insert(__id, inputdeviceObj);
    inputdeviceObj->registerKey ( __id, collection_, s.kernelkeyname, repeat );
    qDebug() << "Register event" << kernelkeyname << inputdeviceObj->device()->devPath << __id;
}

void plugin::unregister_event ( const QString& eventid) {
    const QByteArray eventID = eventid.toAscii();
    m_events.remove(eventID);
    InputDevice* inputdevice = m_devices_by_eventsids.take ( eventID );
    if ( inputdevice )
        inputdevice->unregisterKey (eventID );
}

void plugin::session_change ( int sessionid, bool running ) {
    if (running) {
        foreach ( InputDevice* device, m_devices ) {
            device->disconnectSession ( sessionid );
            device->connectSession ( sessionid );
        }
    } else {
        foreach ( InputDevice* device, m_devices ) {
            device->disconnectSession ( sessionid );
        }
    }
}

void plugin::requestProperties(int sessionid) {
    changeProperty( SceneDocument::createModelReset ( "inputdevice", "udid" ).getData(), sessionid );
    foreach ( InputDevice* device, m_devices ) {
        changeProperty( createServiceOfDevice ( device->device() ).getData(), sessionid );
    }
}

void plugin::deviceAdded ( ManagedDevice* device ) {
    changeProperty ( createServiceOfDevice ( device ).getData() );
    InputDevice* inputdevice = m_devices.value ( device->udid );
    if ( inputdevice )
        return;

    inputdevice = new InputDevice ( this );
    m_devices[device->udid] = inputdevice;
    inputdevice->setDevice ( device );
    QMap<QByteArray, EventInputStructure>::iterator i = m_events.begin();
    for (;i!=m_events.end(); ++i) {
        if (i.value().inputdevice == device->udid)
            inputdevice->registerKey ( i.key(), i.value().collectionuid, i.value().kernelkeyname, i.value().repeat );
    }
    qDebug() << "Device added" << device->devPath;
}

void plugin::deviceRemoved ( ManagedDevice* device ) {
    SceneDocument sc = SceneDocument::createModelRemoveItem ( "inputdevice" );
    sc.setData ( "udid", device->udid );
    changeProperty ( sc.getData() );
	
	qDebug() << "Device removed" << device->devPath;
    delete m_devices.take ( device->udid );
}

SceneDocument plugin::createServiceOfDevice ( ManagedDevice* device ) {
    SceneDocument sc = SceneDocument::createModelChangeItem ( "inputdevice" );
    sc.setData ( "path", device->devPath );
    sc.setData ( "info", device->info );
    sc.setData ( "udid", device->udid );
    return sc;
}






InputDevice::InputDevice ( plugin* plugin ) : m_plugin ( plugin ), m_socketnotifier ( 0 ), fd ( 0 ), m_device ( 0 ) {
    m_stopRepeatTimer.setSingleShot ( true );
    m_stopRepeatTimer.setInterval(150);
}

InputDevice::~InputDevice() {
    qDeleteAll(m_keyToUids);
    m_keyToUids.clear();
    disconnectDevice();
}

ManagedDevice* InputDevice::device() {
    return m_device;
}

bool InputDevice::isClosable() {
    return ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() );
}

void InputDevice::connectSession ( int sessionid ) {
    m_sessionids.insert ( sessionid );
    if ( fd ) {
        SceneDocument sc = SceneDocument::createNotification ( "input.device.selected" );
        sc.setData ( "udid", m_device->udid );
        sc.setData ( "listen", true );
        sc.setData ( "errormsg", QString() );
        // Propagate to all interested clients
        foreach ( int sessionid, m_sessionids ) {
            m_plugin->changeProperty ( sc.getData(), sessionid );
        }
        return;
    }
}
void InputDevice::disconnectSession ( int sessionid ) {
    m_sessionids.remove ( sessionid );
    if ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() ) {
        disconnectDevice();
    }
}

void InputDevice::disconnectDevice() {
    if ( fd ) close ( fd );
    fd = 0;
    delete m_socketnotifier;
    m_socketnotifier = 0;
}

void InputDevice::connectDevice() {
    if ( m_device ) {
        // only reconnect to new device if a client is actually listening or events are registered
        if ( (m_sessionids.isEmpty() && m_keyToUids.isEmpty()) || m_socketnotifier) return;

        SceneDocument sc = SceneDocument::createNotification ( "input.device.selected" );
        sc.setData ( "udid", m_device->udid );

        if ( !QFileInfo ( m_device->devPath ).isReadable() ) {
            // error
            sc.setData ( "listen", false );
            sc.setData ( "errormsg", QString ( QLatin1String ( "InputDevice " ) + m_device->devPath + QLatin1String ( " open failed. No access rights!" ) ) );
        } else {
            fd = open ( m_device->devPath, O_RDONLY|O_NDELAY );
            if ( fd!=-1 ) {
				if (!m_plugin->m_dontgrab)
					ioctl(fd, EVIOCGRAB, 1);
                m_socketnotifier = new QSocketNotifier ( fd, QSocketNotifier::Read );
                connect ( m_socketnotifier, SIGNAL ( activated ( int ) ), this, SLOT ( eventData() ) );
                // success
                sc.setData ( "listen", true );
                sc.setData ( "errormsg", QString() );
            } else {
                sc.setData ( "listen", false );
                sc.setData ( "errormsg", QString ( QLatin1String ( "InputDevice open failed: " ) + m_device->devPath ) );
            }
        }
        // Propagate to all interested clients
        foreach ( int sessionid, m_sessionids ) {
            m_plugin->changeProperty ( sc.getData(), sessionid );
        }
    }
}

void InputDevice::setDevice ( ManagedDevice* device ) {
    m_device = device;
    disconnectDevice();
    connectDevice();
}

void InputDevice::unregisterKey ( const QByteArray& uid ) {
    QMutableMapIterator<QByteArray, EventKey* > it ( m_keyToUids );
    while ( it.hasNext() ) {
        it.next();
        it.value()->ServiceUidToCollectionUid.remove ( uid );
        if ( it.value()->ServiceUidToCollectionUid.isEmpty() )
            it.remove();
    }
    // disconnect if no one is listening anymore
    if ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() )
        disconnectDevice();
}

void InputDevice::registerKey ( QString uid, QString collectionuid, const QByteArray& key, bool repeat ) {
    EventKey* eventkey = m_keyToUids[key];
    if (!eventkey)
		eventkey = new EventKey();
    eventkey->ServiceUidToCollectionUid.insert ( uid, collectionuid );
    eventkey->repeat = repeat;
    m_keyToUids[key] = eventkey;
    connectDevice();
}

void InputDevice::eventData() {
    static char readbuff[sizeof ( struct input_event ) ] = {0};
    static struct input_event* ev = ( struct input_event* ) readbuff;
    static unsigned int readbuffOffset = 0;
    __u16 lastkeyInLoop = 0;
    forever {
        int ret = read ( fd, readbuff+readbuffOffset, sizeof ( struct input_event )-readbuffOffset );
        if ( ret == -1 || ret == 0) break;
        readbuffOffset += ret;
        if ( readbuffOffset < sizeof ( struct input_event ) ) break;
        readbuffOffset = 0;

        if ( ev->type != EV_KEY )
            continue;

        // Only look at KEY_PRESS
        if (!ev->value == KEY_PRESS )
            continue;

        // Multiple key events with the same key in one buffer; ignore
        if (lastkeyInLoop == ev->code)
            continue;
        lastkeyInLoop = ev->code;

        const QByteArray& kernelkeyname = m_plugin->m_keymapping.value ( ev->code );

        //qDebug() << "KEY_PRESS" << kernelkeyname;

        // properties
        if (m_sessionids.size()) {
            // last key property. Will be propagated to interested clients only.
            SceneDocument sc = SceneDocument::createNotification ( "input.device.key" );
            sc.setData ( "kernelkeyname", kernelkeyname );
            sc.setData ( "udid", m_device->udid );

            // Propagate to all interested clients
            foreach ( int sessionid, m_sessionids ) {
                m_plugin->changeProperty ( sc.getData(), sessionid );
            }
        }

        // Get EventKey, abort if no one is registered
        QMap<QByteArray, EventKey*>::iterator it = m_keyToUids.find ( kernelkeyname );
        if ( it == m_keyToUids.end() ) continue;

        const EventKey* event = *it;

        // If received key is the same as last key && the event is not repeatable &&
        // a sensible amount of time to the last received key is not exceeded: abort
        if (m_stopRepeatTimer.isActive() && !event->repeat && kernelkeyname == m_lastkey) {
            // Restart timer to filter out all ongoing events for this key (if repeat==false)
            m_stopRepeatTimer.stop();
            m_stopRepeatTimer.start();
            continue;
        }

        m_stopRepeatTimer.start();
        m_lastkey = kernelkeyname;

        //qDebug() << "KEY_PRESS FILTERED" << kernelkeyname;

        // event trigger
        {
            QMap<QString, QString>::const_iterator i = event->ServiceUidToCollectionUid.constBegin();
            for (;i!=event->ServiceUidToCollectionUid.constEnd();++i) {
                m_plugin->eventTriggered ( i.key(), i.value() );
            }
        }
    }
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}
