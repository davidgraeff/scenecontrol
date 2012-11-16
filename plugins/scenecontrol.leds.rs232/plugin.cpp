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

#include "plugin.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if(!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid) {
	connect(&m_leds, SIGNAL(ledChanged ( QString, int)), SLOT(ledChanged ( QString, int)));
	connect(&m_leds, SIGNAL(curtainChanged ( int, int)), SLOT(curtainChanged ( int, int)));
	connect(&m_leds, SIGNAL(connectedToLeds ( unsigned char )), SLOT(connectedToLeds ( unsigned char )));
}
plugin::~plugin() {}

void plugin::clear() {
	m_leds.disconnectLeds();
	
	changeProperty(SceneDocument::createModelReset("scenecontrol.leds.rs232.values", "channel").getData());
	
    SceneDocument doc;
    doc.setMethod("clear");
	doc.setComponentID(m_pluginid);
	doc.setInstanceID(m_instanceid);
    callRemoteComponentMethod("scenecontrol.ledsnull", doc.getData());
}

void plugin::initialize() {}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
	if (data.contains(QLatin1String("serialport")))
		m_leds.connectToLeds ( data[QLatin1String("serialport")].toString());
}

bool plugin::isLedValue( const QString& channel, int lower, int upper ) {
    const int v = getLed ( channel );
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

void plugin::requestProperties(int sessionid) {
	changeProperty(SceneDocument::createModelReset("scenecontrol.leds.rs232.values", "channel").getData(), sessionid);

	QMap<QString, rs232leds::ledchannel>::iterator i = m_leds.m_leds.begin();
    for (;i!=m_leds.m_leds.end();++i) {
        {
			SceneDocument sc = SceneDocument::createModelChangeItem("scenecontrol.leds.rs232.values");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            changeProperty(sc.getData(), sessionid);
        }
    }
}

void plugin::ledChanged(QString channel, int value) {
	SceneDocument sc = SceneDocument::createModelChangeItem("scenecontrol.leds.rs232.values");
    sc.setData("channel", channel);
    if (value != -1) sc.setData("value", value);
    changeProperty(sc.getData());

	SceneDocument doc;
	doc.setMethod("subpluginChange");
	doc.setComponentID(m_pluginid);
	doc.setInstanceID(m_instanceid);
	doc.setData("channel",channel);
	doc.setData("value",value);
	doc.setData("name",QString());
	callRemoteComponentMethod("scenecontrol.ledsnull", doc.getData());
}

int plugin::getLed ( const QString& channel ) const {
    return m_leds.m_leds.value ( channel ).value;
}

void plugin::toggleLed ( const QString& channel, int fade ) {
	if (! m_leds.channelExist(channel) ) return;
	const unsigned int newvalue = 255 - m_leds.m_leds[channel].value;
    setLed ( channel, newvalue, fade );
}

void plugin::setLedExponential ( const QString& channel, int multiplikator, int fade ) {
	if (! m_leds.channelExist(channel) ) return;
	unsigned int v = m_leds.m_leds[channel].value;
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
	if (! m_leds.channelExist(channel) ) return;
	setLed ( channel,  value + getLed(channel), fade );
}


void plugin::setLed ( const QString& channel, int value, int fade ) {
   m_leds.setLed(channel, value, fade);
}

void plugin::curtainChanged(int /*current*/, int /*max*/) {}

void plugin::connectedToLeds(unsigned char protocolversion) {
	qDebug() << "Connected. Protocol version:" << protocolversion;
}

