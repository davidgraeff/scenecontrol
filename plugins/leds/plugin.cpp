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
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

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

void plugin::clear() {

}

void plugin::initialize() {
    m_ios.clear();
    m_cache.clear();
}

bool plugin::isValue ( const QString& channel, int value )  {
    return ( getLed ( channel ) == value );
}

void plugin::requestProperties(int sessionid) {
    changeProperty(ServiceData::createModelReset("leds", "channel").getData(), sessionid);
    QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
    for (;i!=m_ios.end();++i) {
        const plugin::iochannel& io = i.value();
        ServiceData sc = ServiceData::createModelChangeItem("leds");
        sc.setData("channel", io.channel);
        sc.setData("value", io.value);
        sc.setData("name", io.name);
        sc.setData("moodlight", io.moodlight);
        changeProperty(sc.getData(), sessionid);
    }
}

int plugin::getLed(const QString& channel) const
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
    ServiceData sc = ServiceData::createModelChangeItem("leds");
    sc.setData("channel", channel);
    sc.setData("name", name);
    sc.setData("value", io.value);
    sc.setData("moodlight", io.moodlight);
    changeProperty(sc.getData());

    if (updateDatabase) {
        // save name to database
        QVariantMap settings;
        settings[QLatin1String("channel")] = channel;
        settings[QLatin1String("name")] = name;
        settings[QLatin1String("isname")] = true;
        changeConfig("channelname_" + channel.toUtf8(),settings);
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
void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
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
    QVariantMap datamap;
    for (;it != m_cache.constEnd(); ++it) {
        datamap[QLatin1String("channel")] = (*it)->channel;
        datamap[QLatin1String("value")] = (*it)->value;
        sendDataToPlugin((*it)->plugin_id, datamap);
    }
    m_cache.clear();
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data)
{
    if (ServiceData::isMethod(data, "clear")) {
        // Remove all leds referenced by "plugin_id"
        QMutableMapIterator<QString, iochannel> i(m_ios);
        while (i.hasNext()) {
            i.next();
            if (i.value().plugin_id == plugin_id) {
                ServiceData sc = ServiceData::createModelRemoveItem("leds");
                sc.setData("channel", i.value().channel);
                changeProperty(sc.getData());
                i.remove();
            }
        }
        return;
    }

    if (!data.contains(QLatin1String("channel")) ||
            !data.contains(QLatin1String("value"))
       ) {
        qWarning() << pluginid() << "DataFromPlugin expected channel, name, value" << data;
        return;
    }

    // Assign data to structure
    bool before = m_ios.contains(QLatin1String("channel"));
    iochannel& io = m_ios[data[QLatin1String("channel")].toString()];
    io.plugin_id = plugin_id;
    //p.moodlight = false;
    //p.fadeType = 1;
    io.channel = data[QLatin1String("channel")].toString();
    io.value = data[QLatin1String("value")].toInt();
    if (data.contains(QLatin1String("name")))
        io.name = data[QLatin1String("name")].toString();
    else if (!before)
        io.name = m_namecache.value(io.channel);

    ServiceData sc = ServiceData::createModelChangeItem("leds");
    sc.setData("channel", io.channel);
    if (io.name.size()) sc.setData("name", io.name);
    if (io.value != -1) sc.setData("value", io.value);
    sc.setData("moodlight", io.moodlight);

    changeProperty(sc.getData());
}

