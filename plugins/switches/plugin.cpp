/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010-2012  David Gräff
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

#include "plugin.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    connect(&m_cacheTimer, SIGNAL(timeout()), SLOT(cacheToDevice()));
    m_cacheTimer.setInterval(50);
    m_cacheTimer.setSingleShot(true);
}

plugin::~plugin() {
}

void plugin::clear() {}
void plugin::initialize() {
    m_ios.clear();
    m_ios.clear();
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "switch.value" ) ) {
        setSwitch ( DATA("channel"),BOOLDATA("value") );
    } else if ( ServiceID::isMethod(data, "switch.toogle" ) ) {
        toggleSwitch ( DATA("channel") );
    } else if ( ServiceID::isMethod(data, "switch.name" ) ) {
        setSwitchName ( DATA("channel"),DATA("name") );
    } else if ( ServiceID::isMethod(data, "reload" ) ) {
        initialize();
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "switch.condition" ) ) {
        return ( getSwitch ( DATA("channel") ) == BOOLDATA("value") );
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED ( collectionuid );
    Q_UNUSED ( data );
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED(eventid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
        l.append(ServiceCreation::createModelReset(PLUGIN_ID, "switches", "channel").getData());
        QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
        for (;i!=m_ios.end();++i) {
            const plugin::iochannel str = i.value();
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "switches");
            sc.setData("channel", i.key());
            sc.setData("value", str.value);
            sc.setData("name", str.name);
            l.append(sc.getData());
        }
    }
    return l;
}

bool plugin::getSwitch(const QString& channel) const
{
    if (!m_ios.contains(channel)) return false;
    return m_ios[channel].value;
}

void plugin::setSwitch ( const QString& channel, bool value )
{
    if (!m_ios.contains(channel)) return;
    m_cache[channel] = value;

    if (!m_cacheTimer.isActive()) m_cacheTimer.start();
}

void plugin::setSwitchName ( const QString& channel, const QString& name )
{
    if ( !m_ios.contains(channel) ) return;
    if (name.isNull()) return;

    m_ios[channel].name = name;

    // change name property
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "switches");
    sc.setData("channel", channel);
    sc.setData("name", name);
    m_serverPropertyController->pluginPropertyChanged(sc.getData());

    // save name to database
    QVariantMap settings;
    settings[QLatin1String("channel")] = channel;
    settings[QLatin1String("name")] = name;
    settings[QLatin1String("isname")] = true;
    m_serverPropertyController->saveSettings(QString(QLatin1String("channelname_%1")).arg(channel),settings);
}

void plugin::toggleSwitch ( const QString& pin )
{
    if (!m_ios.contains(pin)) return;
    setSwitch ( pin, !m_ios[pin].value );
}

int plugin::countSwitchs() {
    return m_ios.size();
}

QString plugin::getSwitchName(const QString& pin) {
    if (!m_ios.contains(pin)) return QString();
    return m_ios[pin].name;
}

// Get names from couchdb settings
void plugin::settingsChanged(const QVariantMap& data) {
    Q_UNUSED(data);
}

// send data from set-cache (max 50ms old) to the respective plugin via interconnect communication
void plugin::cacheToDevice()
{
    QMap<QString, unsigned char>::const_iterator it = m_cache.constBegin();
    for (;it != m_cache.constEnd(); ++it) {
        //SENDEN
        QByteArray str;
//TODO
    }
}

plugin::dataFromPlugin(const QByteArray& plugin_id, const QByteArray& data)
{
    const iochannel& p = m_ios[channel];
//TODO

    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "switches");
    sc.setData("channel", channel);
    if (!name.isNull()) sc.setData("name", name);
    if (value != -1) sc.setData("value", value);

    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}
