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

plugin::plugin() : m_events ( QLatin1String ( "inputdevice" ) ) {
    m_devicelist = new ManagedDeviceList();
    connect ( m_devicelist,SIGNAL ( deviceAdded ( ManagedDevice* ) ),SLOT ( deviceAdded ( ManagedDevice* ) ) );
    connect ( m_devicelist,SIGNAL ( deviceRemoved ( ManagedDevice* ) ),SLOT ( deviceRemoved ( ManagedDevice* ) ) );
    _config ( this );
}

plugin::~plugin() {
    qDeleteAll(m_devices);
    delete m_devicelist;
}

void plugin::clear() {}
void plugin::initialize() {
    m_serverPropertyController->pluginRegisterPropertyChangeListener ( QLatin1String ( "selected_input_device" ) );
    for ( int i=0;keynames[i].name;++i ) {
        m_keymapping[keynames[i].value] = QString::fromAscii ( keynames[i].name );
    }
    m_devicelist->start();
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if ( name == QLatin1String ( "repeat" ) ) m_repeat = value.toInt();
    else if ( name == QLatin1String ( "repeat_init" ) ) m_repeatInit = value.toInt();
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    if ( ServiceID::isMethod ( data, "selected_input_device" ) && m_sessions.contains ( sessionid ) ) {
        QMap<QString, InputDevice*>::iterator it = m_devices.begin();
        for ( ;it != m_devices.end();++it ) {
            ( *it )->disconnectSession ( sessionid );
        }
        InputDevice* inputdevice = m_devices.value ( DATA ( "udid" ) );
        if ( !inputdevice ) return;
        inputdevice->connectSession ( sessionid );
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    Q_UNUSED ( data );
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    if ( ServiceID::isMethod ( data, "inputevent" ) ) {
        m_events.add ( data, collectionuid );
        InputDevice* inputdevice = m_devices.value ( DATA ( "inputdevice" ) );
        if ( inputdevice ) inputdevice->registerKey ( ServiceID::id ( data ), collectionuid, DATA ( "kernelkeyname" ), BOOLDATA ( "repeat" ) );
    }
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(sessionid);
    const QVariantMap& data = m_events.remove ( eventid );
    InputDevice* inputdevice = m_devices.value ( DATA ( "inputdevice" ) );
    if ( inputdevice ) inputdevice->unregisterKey (eventid );
}

void plugin::session_change ( int sessionid, bool running ) {
    PluginSessionsHelper::session_change ( sessionid, running );
    if ( running ) return;
    foreach ( InputDevice* device, m_devices ) {
        device->disconnectSession ( sessionid );
    }
}

QList<QVariantMap> plugin::properties ( int sessionid ) {
    Q_UNUSED ( sessionid );
    QList<QVariantMap> l;
    l.append ( ServiceCreation::createModelReset ( PLUGIN_ID, "inputdevice", "udid" ).getData() );
    foreach ( InputDevice* device, m_devices ) {
        l.append ( createServiceOfDevice ( device->device() ).getData() );
    }
    return l;
}

void plugin::deviceAdded ( ManagedDevice* device ) {
    m_serverPropertyController->pluginPropertyChanged ( createServiceOfDevice ( device ).getData() );
    InputDevice* inputdevice = m_devices.value ( device->udid );
    if ( !inputdevice ) {
        inputdevice = new InputDevice ( this );
        m_devices[device->udid] = inputdevice;
    }
    inputdevice->setDevice ( device );
    const QList<QVariantMap> datas = m_events.data(device->udid);
    foreach(QVariantMap data, datas) {
        inputdevice->registerKey ( ServiceID::id ( data ), ServiceID::collectionid(data), DATA ( "kernelkeyname" ), BOOLDATA ( "repeat" ) );
    }
}

void plugin::deviceRemoved ( ManagedDevice* device ) {
    ServiceCreation sc = ServiceCreation::createModelRemoveItem ( PLUGIN_ID, "inputdevice" );
    sc.setData ( "udid", device->udid );
    m_serverPropertyController->pluginPropertyChanged ( sc.getData() );
    delete m_devices.take ( device->udid );
}

ServiceCreation plugin::createServiceOfDevice ( ManagedDevice* device ) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem ( PLUGIN_ID, "inputdevice" );
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
        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "input.device.selected" );
        sc.setData ( "udid", m_device->udid );
        sc.setData ( "listen", true );
        sc.setData ( "errormsg", QString() );
        // Propagate to all interested clients
        foreach ( int sessionid, m_sessionids ) {
            m_plugin->m_serverPropertyController->pluginPropertyChanged ( sc.getData(), sessionid );
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

        ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "input.device.selected" );
        sc.setData ( "udid", m_device->udid );

        if ( !QFileInfo ( m_device->devPath ).isReadable() ) {
            // error
            sc.setData ( "listen", false );
            sc.setData ( "errormsg", QString ( QLatin1String ( "InputDevice " ) + m_device->devPath + QLatin1String ( " open failed. No access rights!" ) ) );
        } else {
            fd = open ( m_device->devPath.toUtf8(), O_RDONLY|O_NDELAY );
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
            m_plugin->m_serverPropertyController->pluginPropertyChanged ( sc.getData(), sessionid );
        }
    }
}

void InputDevice::setDevice ( ManagedDevice* device ) {
    m_device = device;
    disconnectDevice();
    connectDevice();
}

void InputDevice::unregisterKey ( QString uid ) {
    QMutableMapIterator<QString, EventKey* > it ( m_keyToUids );
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
void InputDevice::registerKey ( QString uid, QString collectionuid, QString key, bool repeat ) {
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
            const QString& kernelkeyname = m_plugin->m_keymapping.value ( ev->code );
            //qDebug() << "key event" << m_device->devPath << kernelkeyname;
            // properties
            {
                // last key property. Will be propagated to interested clients only.
                ServiceCreation sc = ServiceCreation::createNotification ( PLUGIN_ID, "input.device.key" );
                sc.setData ( "kernelkeyname", kernelkeyname );
                sc.setData ( "udid", m_device->udid );

                // Propagate to all interested clients
                foreach ( int sessionid, m_sessionids ) {
                    m_plugin->m_serverPropertyController->pluginPropertyChanged ( sc.getData(), sessionid );
                }
            }
            m_lastkey = kernelkeyname;
            repeattrigger ( true );
        }
    }
    m_socketnotifier->setEnabled(true);
}
void InputDevice::repeattrigger ( bool initial_event ) {
    QMap<QString, EventKey*>::iterator it = m_keyToUids.find ( m_lastkey );
    if ( it == m_keyToUids.end() ) return;

    const EventKey* event = *it;
    QMap<QString, QString>::const_iterator i = event->ServiceUidToCollectionUid.constBegin();
    for (;i!=event->ServiceUidToCollectionUid.constEnd();++i) {
        m_plugin->m_serverCollectionController->pluginEventTriggered ( i.key(), i.value() );
    }

    if ( event->repeat )
        m_repeattimer.start ( initial_event?m_plugin->m_repeatInit:m_plugin->m_repeat );
}

