/*/*
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
#include "shared/pluginhelper.h"
#include <QSet>
#include <QTimer>

class plugin : public QObject, public PluginHelper
{
    Q_OBJECT
    PLUGIN_MACRO
    Q_INTERFACES(AbstractPlugin)
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual QMap<QString, QVariantMap> properties();
    virtual void otherPropertyChanged(const QString& unqiue_property_id, const QVariantMap& value, const QString& sessionid);
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false);
    virtual void execute(const QVariantMap& data);
    virtual bool condition(const QVariantMap& data) ;
    virtual void event_changed(const QVariantMap& data);
private:
	// input events
	QMap<QString, QSet<QString> > m_key_events; //device+key->set of uids
	QString m_lastevent;
	void inputEvent(const QString& device, const QString& kernelkeyname, bool pressed);
	// devices
	QSet<QString> m_alldevices;
	void listenToDevice(const QString& device);
	void stoplistenToDevice(const QString& device);
	// repeat
    QTimer m_repeattimer;
    bool m_dorepeat;
private Q_SLOTS:
	void inputDevicesChanged();
	void repeattrigger();
};
