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
    if (argc<4) {
		qWarning()<<"Usage: instance_id server_ip server_port";
		return 1;
	}
    
    if (AbstractPlugin::createInstance<plugin>(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 10;
    return app.exec();
}

plugin::~plugin() {
}

void plugin::clear() {

}

void plugin::clear(const QString& componentid_, const QString& instanceid_) {
    // Remove all leds referenced by "plugin_id"
    QMutableMapIterator<QString, iochannel> i(m_ios);
    while (i.hasNext()) {
        i.next();
		if (i.value().componentID == componentid_ && i.value().instanceID == instanceid_) {
            SceneDocument sc = SceneDocument::createModelRemoveItem("leds");
            sc.setData("channel", i.value().channel);
            changeProperty(sc.getData());
            i.remove();
        }
    }
}

void plugin::initialize() {
	connect(&m_moodlightTimer, SIGNAL(timeout()),SLOT(moodlightTimeout()));
	m_moodlightTimer.setInterval(5000);
	srand(100);
    m_ios.clear();
}

bool plugin::isLedValue ( const QString& channel, int lower, int upper )  {
    const int v = getLed ( channel );
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

void plugin::requestProperties() {
	changeProperty(SceneDocument::createModelReset("leds", "channel").getData(), m_lastsessionid);
    QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
    for (;i!=m_ios.end();++i) {
        const plugin::iochannel& io = i.value();
        SceneDocument sc = SceneDocument::createModelChangeItem("leds");
        sc.setData("channel", io.channel);
        sc.setData("value", io.value);
        sc.setData("name", io.name);
        sc.setData("moodlight", io.moodlight);
		changeProperty(sc.getData(), m_lastsessionid);
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

void plugin::setLedExponential ( const QString& channel, int multiplikator, int fade ) {
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

void plugin::setLedRelative ( const QString& channel, int value, int fade ) {
    if (! m_ios.contains(channel) ) return;
    setLed ( channel,  value + m_ios[channel].value, fade );
}

void plugin::setLed ( const QString& channel, int value, int fade )
{
    if (!m_ios.contains(channel)) return;
    iochannel& p = m_ios[channel];
    p.value = value;
    p.fadeType = fade;

    SceneDocument doc;
	doc.setComponentID(p.componentID);
	doc.setInstanceID(p.instanceID);
    doc.setMethod("setLed");
	doc.setData("channel",channel);
	doc.setData("value",value);
	doc.setData("fade",fade);
	callRemoteComponent(doc.getData());
}

void plugin::setLedName ( const QString& channel, const QString& name, bool updateDatabase )
{
    if ( !m_ios.contains(channel) ) return;
    if (name.isNull()) return;

    plugin::iochannel& io = m_ios[channel];

    io.name = name;

    // change name property
    SceneDocument sc = SceneDocument::createModelChangeItem("leds");
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

void plugin::toggleLed ( const QString& channel, int fade )
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

// Get names from database
void plugin::instanceConfiguration(const QVariantMap& data) {
    
    if (data.contains(QLatin1String("isname")) && data.contains(QLatin1String("channel")) && data.contains(QLatin1String("name"))) {
        const QByteArray channel = data.value(QLatin1String("channel")).toByteArray();
        const QString name = data.value(QLatin1String("name")).toString();
        if (m_ios.contains(channel)) {
            m_ios[channel].name = name;
        }
        m_namecache.insert(channel, name);
    }
}

void plugin::subpluginChange(const QString& componentid_, const QString& instanceid_, const QString& channel, int value, const QString& name) {
    // Assign data to structure
    bool before = m_ios.contains(channel);
    iochannel& io = m_ios[channel];
	io.componentID = componentid_.toUtf8();
	io.instanceID = instanceid_.toUtf8();
    //p.moodlight = false;
    //p.fadeType = 1;
    io.channel = channel;
    io.value = value;
    if (!before)
        io.name = name.size()?name : m_namecache.value(io.channel);
    if (io.name.isEmpty()) {
        io.name = QLatin1String("Channel ") + QString::number(m_ios.size());
    }

    SceneDocument sc = SceneDocument::createModelChangeItem("leds");
    sc.setData("channel", io.channel);
    if (io.name.size()) sc.setData("name", io.name);
    if (io.value != -1) sc.setData("value", io.value);
    sc.setData("moodlight", io.moodlight);

    changeProperty(sc.getData());
}

