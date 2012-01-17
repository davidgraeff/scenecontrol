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
    m_cache.clear();
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
            const plugin::iochannel& str = i.value();
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "switches");
            sc.setData("channel", str.channel);
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
    iochannel& p = m_ios[channel];
    p.value = value;
    m_cache.insert(&p);

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
    m_serverPropertyController->saveSettings(QString(QLatin1String("channelname_%1")).arg(channel),settings, PLUGIN_ID);
}

void plugin::toggleSwitch ( const QString& channel )
{
    if (!m_ios.contains(channel)) return;
    setSwitch ( channel, !m_ios[channel].value );
}

int plugin::countSwitchs() {
    return m_ios.size();
}

QString plugin::getSwitchName(const QString& channel) {
    if (!m_ios.contains(channel)) return QString();
    return m_ios[channel].name;
}

// Get names from couchdb settings
void plugin::settingsChanged(const QVariantMap& data) {
    Q_UNUSED(data);
}

// send data from set-cache (max 50ms old) to the respective plugin via interconnect communication
void plugin::cacheToDevice()
{
    QSet<iochannel*>::const_iterator it = m_cache.constBegin();
    for (;it != m_cache.constEnd(); ++it) {
        sendDataToPlugin((*it)->plugin_id, (*it)->channel.toAscii() + "\t" + QByteArray::number((*it)->value));
    }
    m_cache.clear();
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QByteArray& data)
{
    const QList<QByteArray> t = data.split('\t');
    // t[0]: channel
    // t[1]: value
    // t[2]: name
    if (t.size() < 2) {
        if (t[0] == "CLEAR") {
            m_serverPropertyController->pluginPropertyChanged(sc.getData());
            QMutableMapIterator<QString, iochannel> i(m_ios);
            while (i.hasNext()) {
                i.next();
                if (i.value().plugin_id = plugin_id) {
                    ServiceCreation sc = ServiceCreation::createModelRemoveItem(PLUGIN_ID, "switches");
                    sc.setData("channel", i.value().channel);
                    m_serverPropertyController->pluginPropertyChanged(sc.getData());
                    i.remove();
                }
            }
            return;
        }
        qWarning() << pluginid() << "DataFromPlugin expected >= 2 data blocks";
        return;
    }

    // Assign data to structure
    iochannel& io = m_ios[QString::fromAscii(t[0])];
    io.channel = QString::fromAscii(t[0]);
    io.value = t[1].toInt();
    io.plugin_id = plugin_id;
    if (t.size()>2) io.name = QString::fromAscii(t[2]);

    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "switches");
    sc.setData("channel", io.channel);
    if (!io.name.isNull()) sc.setData("name", io.name);
    if (io.value != -1) sc.setData("value", io.value);

    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}
