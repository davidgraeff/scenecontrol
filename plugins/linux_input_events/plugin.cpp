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
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>
#include <sys/ioctl.h>

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
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() : AbstractPlugin(this) {
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
}

void plugin::select_input_device ( int sessionid, const QByteArray& udid) {
    QMap<QByteArray, InputDevice*>::iterator it = m_devices.begin();
    for ( ;it != m_devices.end();++it ) {
        ( *it )->disconnectSession ( sessionid );
    }
    InputDevice* inputdevice = m_devices.value ( udid );
    if ( !inputdevice ) return;
    inputdevice->connectSession ( sessionid );
}

void plugin::inputevent ( const QByteArray& _id, const QByteArray& collection_, const QByteArray& inputdevice, const QByteArray& kernelkeyname, bool repeat) {
    // Add to input events list
    EventInputStructure s;
    s.collectionuid = collection_;
    s.inputdevice = inputdevice;
    s.kernelkeyname = kernelkeyname;
    s.repeat = repeat;
    m_events.insert(_id, s);

    // If device for this event already exists, register key
    InputDevice* inputdeviceObj = m_devices.value ( inputdevice );
    qDebug() << "inputevent"; // << inputdevice << inputdeviceObj;
    if ( !inputdeviceObj )
        return;
    m_devices_by_eventsids.insert(_id, inputdeviceObj);
    inputdeviceObj->registerKey ( _id, collection_, kernelkeyname, repeat );
}

void plugin::unregister_event ( const QString& eventid) {
  const QByteArray eventID2 = eventid.toAscii();
    m_events.remove(eventID2);
    InputDevice* inputdevice = m_devices_by_eventsids.take ( eventID2 );
    if ( inputdevice )
        inputdevice->unregisterKey (eventID2 );
}

void plugin::session_change ( int sessionid, bool running ) {
    if ( running ) return;
    foreach ( InputDevice* device, m_devices ) {
        device->disconnectSession ( sessionid );
    }
}

void plugin::requestProperties(int sessionid) {
    changeProperty( ServiceData::createModelReset ( "inputdevice", "udid" ).getData(), sessionid );
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
}

void plugin::deviceRemoved ( ManagedDevice* device ) {
    ServiceData sc = ServiceData::createModelRemoveItem ( "inputdevice" );
    sc.setData ( "udid", device->udid );
    changeProperty ( sc.getData() );
    delete m_devices.take ( device->udid );
}

ServiceData plugin::createServiceOfDevice ( ManagedDevice* device ) {
    ServiceData sc = ServiceData::createModelChangeItem ( "inputdevice" );
    sc.setData ( "path", device->devPath );
    sc.setData ( "info", device->info );
    sc.setData ( "udid", device->udid );
    return sc;
}

InputDevice::InputDevice ( plugin* plugin ) : m_plugin ( plugin ), m_socketnotifier ( 0 ), fd ( 0 ), m_device ( 0 ) {
    connect ( &m_repeattimer,SIGNAL ( timeout() ),SLOT ( repeattrigger() ) );
    m_repeattimer.setSingleShot ( true );
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
        ServiceData sc = ServiceData::createNotification ( "input.device.selected" );
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
        if ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() ) return;

        ServiceData sc = ServiceData::createNotification ( "input.device.selected" );
        sc.setData ( "udid", m_device->udid );

        if ( !QFileInfo ( m_device->devPath ).isReadable() ) {
            // error
            sc.setData ( "listen", false );
            sc.setData ( "errormsg", QString ( QLatin1String ( "InputDevice " ) + m_device->devPath + QLatin1String ( " open failed. No access rights!" ) ) );
        } else {
            fd = open ( m_device->devPath, O_RDONLY|O_NDELAY );
            if ( fd!=-1 ) {
                delete m_socketnotifier;
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
    if (!eventkey) eventkey = new EventKey();
    eventkey->ServiceUidToCollectionUid.insertMulti ( uid, collectionuid );
    eventkey->repeat = repeat;
    m_keyToUids[key] = eventkey;
    connectDevice();
}

void InputDevice::eventData() {
    m_socketnotifier->setEnabled(false);
    static char readbuff[sizeof ( struct input_event ) ] = {0};
    static struct input_event* ev = ( struct input_event* ) readbuff;
    static unsigned int readbuffOffset = 0;
    while ( 1 ) {
        int ret = read ( fd, readbuff+readbuffOffset, sizeof ( struct input_event )-readbuffOffset );
        if ( ret == -1 ) break;
        readbuffOffset += ret;
        if ( readbuffOffset < sizeof ( struct input_event ) ) break;
        readbuffOffset = 0;

        if ( ev->type != EV_KEY ) break;
        m_repeattimer.stop();
        if ( ( ev->value == KEY_PRESS ) || ( ev->value == KEY_KEEPING_PRESSED ) ) {
            const QByteArray& kernelkeyname = m_plugin->m_keymapping.value ( ev->code );
            //qDebug() << "key event" << m_device->devPath << kernelkeyname;
            // properties
            {
                // last key property. Will be propagated to interested clients only.
                ServiceData sc = ServiceData::createNotification ( "input.device.key" );
                sc.setData ( "kernelkeyname", kernelkeyname );
                sc.setData ( "udid", m_device->udid );

                // Propagate to all interested clients
                foreach ( int sessionid, m_sessionids ) {
                    m_plugin->changeProperty ( sc.getData(), sessionid );
                }
            }
            m_lastkey = kernelkeyname;
            repeattrigger ( true );
        }
    }
    m_socketnotifier->setEnabled(true);
}
void InputDevice::repeattrigger ( bool initial_event ) {
    QMap<QByteArray, EventKey*>::iterator it = m_keyToUids.find ( m_lastkey );
    if ( it == m_keyToUids.end() ) return;

    const EventKey* event = *it;
    QMap<QString, QString>::const_iterator i = event->ServiceUidToCollectionUid.constBegin();
    for (;i!=event->ServiceUidToCollectionUid.constEnd();++i) {
        m_plugin->eventTriggered ( i.key().toAscii(), i.value().toAscii() );
    }

    if ( event->repeat )
        m_repeattimer.start ( initial_event?m_plugin->m_repeatInit:m_plugin->m_repeat );
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}