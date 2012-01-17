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
    connect(&m_moodlightTimer, SIGNAL(timeout()),SLOT(moodlightTimeout()));
    m_moodlightTimer.setInterval(5000);
    srand(100);
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
    if ( ServiceID::isMethod(data, "led.value" ) ) {
        setLed ( DATA("channel"),INTDATA("value"), INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "led.value_relative" ) ) {
        setLedRelative ( DATA("channel"),INTDATA("value"), INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "led.value_exponential" ) ) {
        setLedExponential ( DATA("channel"),INTDATA("multiplikator"), INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "led.toogle" ) ) {
        toggleLed ( DATA("channel"), INTDATA("fade")  );
    } else if ( ServiceID::isMethod(data, "led.name" ) ) {
        setLedName ( DATA("channel"),DATA("name") );
    } else if ( ServiceID::isMethod(data, "reload" ) ) {
        initialize();
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "led.condition" ) ) {
        return ( getLed ( DATA("channel") ) == INTDATA("value") );
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
        l.append(ServiceCreation::createModelReset(PLUGIN_ID, "leds", "channel").getData());
        QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
        for (;i!=m_ios.end();++i) {
            const plugin::iochannel& io = i.value();
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "leds");
            sc.setData("channel", io.channel);
            sc.setData("value", io.value);
            sc.setData("name", io.name);
            sc.setData("moodlight", io.moodlight);
            l.append(sc.getData());
        }
    }
    return l;
}

bool plugin::getLed(const QString& channel) const
{
    if (!m_ios.contains(channel)) return false;
    return m_ios[channel].value;
}


void plugin::moodlight(const QString& channel, bool moodlight) {
    if ( !m_ios.contains(channel) ) return;
    m_ios[channel].moodlight = moodlight;
    if (moodlight) {
        m_moodlightTimer.start();
        moodlightTimeout();
    }
}

void plugin::setLedExponential ( const QString& channel, int multiplikator, uint fade ) {
    if ( !m_ios.contains(channel) ) return;
    unsigned int v = m_ios[channel].value;
    if ( multiplikator>100 ) {
        if ( v==0 )
            v=1;
        else if ( v==1 )
            v=2;
        else
            v = ( v * multiplikator ) /100+1;
    } else {
        if ( v<2 )
            v = 0;
        else
            v = ( v * multiplikator ) /100-1;
    }

    setLed ( channel, v, fade );
}

void plugin::setLedRelative ( const QString& channel, int value, uint fade ) {
    if (! m_ios.contains(channel) ) return;
    setLed ( channel,  value + m_ios[channel].value, fade );
}

void plugin::setLed ( const QString& channel, int value, uint fade )
{
    if (!m_ios.contains(channel)) return;
    iochannel& p = m_ios[channel];
    p.value = value;
    p.fadeType = fade;
    m_cache.insert(&p);

    if (!m_cacheTimer.isActive()) m_cacheTimer.start();
}

void plugin::setLedName ( const QString& channel, const QString& name, bool updateDatabase )
{
    if ( !m_ios.contains(channel) ) return;
    if (name.isNull()) return;

    plugin::iochannel& io = m_ios[channel];

    io.name = name;

    // change name property
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "leds");
    sc.setData("channel", channel);
    sc.setData("name", name);
    sc.setData("value", io.value);
    sc.setData("moodlight", io.moodlight);
    m_serverPropertyController->pluginPropertyChanged(sc.getData());

    if (updateDatabase) {
        // save name to database
        QVariantMap settings;
        settings[QLatin1String("channel")] = channel;
        settings[QLatin1String("name")] = name;
        settings[QLatin1String("isname")] = true;
        m_serverPropertyController->saveSettings(QString(QLatin1String("channelname_%1")).arg(channel),settings, PLUGIN_ID);
    }
}

void plugin::toggleLed ( const QString& channel, uint fade )
{
    if (!m_ios.contains(channel)) return;
    setLed ( channel, (m_ios[channel].value==0?255:0), fade );
}

int plugin::countLeds() {
    return m_ios.size();
}

QString plugin::getLedName(const QString& channel) {
    if (!m_ios.contains(channel)) return QString();
    return m_ios[channel].name;
}

void plugin::moodlightTimeout() {
    QMap<QString,iochannel>::iterator i = m_ios.begin();
    int c = 0;
    for (;i != m_ios.end();++i) {
        if (!i.value().moodlight) continue;
        ++c;
        if (rand()/RAND_MAX >0.5) continue;
        const uint FADE = 1;
        setLed(i.key(),rand()%255, FADE);
    }
    if (!c) m_moodlightTimer.stop();
}

// Get names from couchdb settings
void plugin::settingsChanged(const QVariantMap& data) {
    if (data.contains(QLatin1String("isname")) && data.contains(QLatin1String("channel")) && data.contains(QLatin1String("name"))) {
        const QString channel = data.value(QLatin1String("channel")).toString();
        const QString name = data.value(QLatin1String("name")).toString();
        if (m_ios.contains(channel)) {
            m_ios[channel].name = name;
        }
        m_namecache.insert(channel, name);


    }
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
    if (data.startsWith("CLEAR")) {
        // Remove all leds referenced by "plugin_id"
        QMutableMapIterator<QString, iochannel> i(m_ios);
        while (i.hasNext()) {
            i.next();
            if (i.value().plugin_id == plugin_id) {
                ServiceCreation sc = ServiceCreation::createModelRemoveItem(PLUGIN_ID, "leds");
                sc.setData("channel", i.value().channel);
                m_serverPropertyController->pluginPropertyChanged(sc.getData());
                i.remove();
            }
        }
        return;
    }

    const QDataStream stream ( data );
    QVariantMap datamap;
    stream >> datamap;

    if (!datamap.contains(QLatin1String("channel")) ||
            !datamap.contains(QLatin1String("value"))
       ) {
        qWarning() << pluginid() << "DataFromPlugin expected channel, name, value" << datamap;
        return;
    }

    // Assign data to structure
    iochannel& io = m_ios[datamap[QLatin1String("channel")].toString()];
    io.plugin_id = plugin_id;
    //p.moodlight = false;
    //p.fadeType = 1;
    io.channel = datamap[QLatin1String("channel")].toString();
    io.value = datamap[QLatin1String("value")].toInt();
    if (datamap.contains(QLatin1String("name")))
        io.name = datamap[QLatin1String("name")].toString();

    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "leds");
    sc.setData("channel", io.channel);
    if (io.name.size()) sc.setData("name", io.name);
    if (io.value != -1) sc.setData("value", io.value);
    sc.setData("moodlight", io.moodlight);

    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}

