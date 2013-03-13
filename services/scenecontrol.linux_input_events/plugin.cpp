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

#include "parse.h"

#include "plugin.h"
#include "managed_device_list.h"
#include "inputdevice.h"
#include <qfileinfo.h>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<4) {
		qWarning()<<"Usage: instance_id server_ip server_port";
		return 1;
	}
    
    if (plugin::createInstance(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 10;
    return app.exec();
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
	delete m_devicelist;
	m_devicelist = 0;
}

void plugin::initialize() {
	m_dontgrab = 0;
	m_repeat = 0;
	m_repeatInit = 0;
	m_devicelist = new ManagedDeviceList();
	connect ( m_devicelist,SIGNAL ( deviceAdded ( ManagedDevice* ) ),SLOT ( deviceAdded ( ManagedDevice* ) ) );
	connect ( m_devicelist,SIGNAL ( deviceRemoved ( ManagedDevice* ) ),SLOT ( deviceRemoved ( ManagedDevice* ) ) );
	
    for ( int i=0;keynames[i].name;++i ) {
        m_keymapping[keynames[i].value] = keynames[i].name;
    }
    m_devicelist->start();
}

void plugin::instanceConfiguration(const QVariantMap& data) {
    
    if (data.contains(QLatin1String("repeat")))
        m_repeat = data[QLatin1String("repeat")].toInt();
    if (data.contains(QLatin1String("repeat_init")))
        m_repeatInit = data[QLatin1String("repeat_init")].toInt();
    if (data.contains(QLatin1String("dontgrab")))
        m_dontgrab = data[QLatin1String("dontgrab")].toBool();
}

void plugin::inputevent ( const QString& id_, const QString& sceneid_, const QString& inputdevice, const QString& kernelkeyname, bool repeat) {
	if (id_.isEmpty()) {
		qWarning() << "Not (un)registering event:" << id_ << sceneid_;
		return;
	}
	if (sceneid_.isEmpty()) {
		const QByteArray eventID = id_.toAscii();
		m_events.remove(eventID);
		InputDevice* inputdevice = m_devices_by_eventsids.take ( eventID );
		qDebug() << "Unregister event" << eventID;
		if ( inputdevice )
			inputdevice->unregisterKey (eventID );
		return;
	}
	QByteArray eventID = id_.toAscii();
    // Add to input events list
    EventInputStructure s;
    s.collectionuid = sceneid_.toAscii();
    s.inputdevice = inputdevice.toAscii();
    s.kernelkeyname = kernelkeyname.toAscii();
    s.repeat = repeat;
	m_events.insert(eventID, s);

    // If device for this event already exists, register key
    InputDevice* inputdeviceObj = m_devices.value ( inputdevice.toAscii() );
    if ( !inputdeviceObj )
        return;
	m_devices_by_eventsids.insert(eventID, inputdeviceObj);
	inputdeviceObj->registerKey ( eventID, sceneid_, s.kernelkeyname, repeat );
	qDebug() << "Register event" << kernelkeyname << inputdeviceObj->device()->devPath << eventID;
}

void plugin::listenToInputEvents(const QString& inputdevice)
{
	if (m_lastsessionid==-1)
		return;
	foreach ( InputDevice* device, m_devices ) {
		device->disconnectSession ( m_lastsessionid );
	}
	InputDevice* inputdeviceObj = m_devices.value ( inputdevice.toAscii() );
	if ( !inputdeviceObj )
		return;
	inputdeviceObj->connectSession ( m_lastsessionid );
}

void plugin::session_change ( bool running ) {
    if (!running) {
        foreach ( InputDevice* device, m_devices ) {
			device->disconnectSession ( m_lastsessionid );
        }
    }
}

void plugin::requestProperties() {
	changeProperty( SceneDocument::createModelReset ( "inputdevice", "udid" ).getData(), m_lastsessionid );
    foreach ( InputDevice* device, m_devices ) {
		changeProperty( createServiceOfDevice ( device->device() ).getData(), m_lastsessionid );
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
