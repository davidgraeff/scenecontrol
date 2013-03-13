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

#pragma once
#include <QObject>
#include <QStringList>
#include "abstractplugin.h"
#include <QSet>
#include <QTimer>
#include <qfile.h>
#include <linux/input.h>
#include <QSocketNotifier>

class InputDevice;
class ManagedDevice;
class ManagedDeviceList;

class plugin : public AbstractPlugin
{
    Q_OBJECT
    friend class InputDevice;
public:

    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties();
    virtual void instanceConfiguration(const QVariantMap& data);
    virtual void session_change ( bool running );

public Q_SLOTS:
  void inputevent ( const QString& id_, const QString& sceneid_, const QString& inputdevice, const QString& kernelkeyname, bool repeat );
  void listenToInputEvents (const QString& inputdevice);
private:
    ManagedDeviceList* m_devicelist;
    // udid -> device
    QMap<QByteArray, InputDevice*> m_devices;
    // eventid -> device
    QMap<QByteArray, InputDevice*> m_devices_by_eventsids;
    struct EventInputStructure {
      QByteArray collectionuid;
      QByteArray inputdevice;
      QByteArray kernelkeyname;
      bool repeat;
    };
    // eventid -> structure for registering a key as soon as the device is available
    QMap<QByteArray, EventInputStructure> m_events;
    
    QMap<uint, QByteArray> m_keymapping;
    
	int m_dontgrab; // if true: do not grab devices exclusivly
    int m_repeat; //NotUsed
    int m_repeatInit; //NotUsed
    SceneDocument createServiceOfDevice(ManagedDevice* device);
private Q_SLOTS:
    void deviceAdded(ManagedDevice*);
    void deviceRemoved(ManagedDevice*);

};
