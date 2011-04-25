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
#include "shared/abstractplugin.h"
#include "shared/abstractserver.h"
#include "shared/pluginsettingshelper.h" 
#include "shared/abstractplugin_services.h"
#include <QSet>
#include <QTimer>
#include <qfile.h>
#include <linux/input.h>
#include <shared/pluginsessionhelper.h>

class plugin;
class ManagedDeviceList;
class ManagedDevice;

struct EventKey {
    QSet<QString> uids;
    bool repeat;
};

struct InputDevice : public QObject {
    Q_OBJECT
private:
    ManagedDevice* m_device;
    QFile m_file;
    QSet<QString> m_sessionids;
    QMap<QString, EventKey*> m_keyToUids;
    QTimer m_repeattimer;
    QString m_lastkey;
    plugin* m_plugin;
public:
    InputDevice(plugin* plugin) ;
    ~InputDevice();
    bool isClosable();
    void connectSession(const QString& sessionid);
    void disconnectSession(const QString& sessionid);
    void setDevice(ManagedDevice* device);
    void unregisterKey(QString uid);
    void registerKey(QString uid, QString key, bool repeat);
	ManagedDevice* device();
private Q_SLOTS:
    void eventData();
    void repeattrigger(bool initial_event = false);
};

class plugin : public QObject, public PluginSettingsHelper, public PluginSessionsHelper, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
    Q_INTERFACES(AbstractPlugin AbstractPlugin_settings AbstractPlugin_services AbstractPlugin_sessions)
    friend class InputDevice;
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual QList<QVariantMap> properties(const QString& sessionid);
    virtual void session_change(const QString& id, bool running);
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false);
    virtual void execute(const QVariantMap& data, const QString& sessionid);
    virtual bool condition(const QVariantMap& data, const QString& sessionid) ;
    virtual void event_changed(const QVariantMap& data, const QString& sessionid);
private:
    ManagedDeviceList* m_devicelist;
    QMap<QString, InputDevice*> m_devices;
	QMap<uint, QString> m_keymapping;

    static int m_repeat;
    static int m_repeatInit;
private Q_SLOTS:
    void deviceAdded(ManagedDevice*);
    void deviceRemoved(ManagedDevice*);

};
