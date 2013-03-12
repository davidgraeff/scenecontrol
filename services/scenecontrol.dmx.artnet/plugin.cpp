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
    if (argc<4) {
		qWarning()<<"Usage: instance_id server_ip server_port";
		return 1;
	}
    
    if (plugin::createInstance(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 2;
    return app.exec();
}

plugin::~plugin() {
    delete m_socket;
}

void plugin::clear() {
    m_connectTimer.stop();
    m_leds.clear();
	
	changeProperty(SceneDocument::createModelReset("udpled.values", "channel").getData());
	
    SceneDocument doc;
    doc.setMethod("clear");
	doc.setComponentID(QLatin1String("scenecontrol.leds"));
	doc.setInstanceID(QLatin1String("null"));
	callRemoteComponent(doc.getData());
}

void plugin::initialize() {
	connect(&m_connectTimer, SIGNAL(timeout()), SLOT(resendConnectSequence()));
	m_connectTimer.setSingleShot(true);
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port")))
        connectToLeds ( data[QLatin1String("server")].toString(), data[QLatin1String("port")].toInt() );
}

bool plugin::isLedValue( const QString& channel, int lower, int upper ) {
    const int v = getLed ( channel );
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

void plugin::requestProperties(int sessionid) {
	changeProperty(SceneDocument::createModelReset("udpled.values", "channel").getData(), sessionid);

    QMap<QString, plugin::ledchannel>::iterator i = m_leds.begin();
    for (;i!=m_leds.end();++i) {
        {
			SceneDocument sc = SceneDocument::createModelChangeItem("udpled.values");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            changeProperty(sc.getData(), sessionid);
        }
    }
}

void plugin::ledChanged(QString channel, int value) {
	SceneDocument sc = SceneDocument::createModelChangeItem("udpled.values");
    sc.setData("channel", channel);
    if (value != -1) sc.setData("value", value);
    changeProperty(sc.getData());

	SceneDocument doc;
	doc.setMethod("subpluginChange");
	doc.setComponentID(QLatin1String("scenecontrol.leds"));
	doc.setInstanceID(QLatin1String("null"));
	doc.setData("channel",channel);
	doc.setData("value",value);
	doc.setData("name",QString());
	callRemoteComponent(doc.getData());
}

int plugin::getLed ( const QString& channel ) const {
    return m_leds.value ( channel ).value;
}

void plugin::toggleLed ( const QString& channel, int fade ) {
    if ( !m_leds.contains(channel) ) return;
    const unsigned int newvalue = 255 - m_leds[channel].value;
    setLed ( channel, newvalue, fade );
}

void plugin::setLedExponential ( const QString& channel, int multiplikator, int fade ) {
    if ( !m_leds.contains(channel) ) return;
    unsigned int v = m_leds[channel].value;
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
    if (! m_leds.contains(channel) ) return;
    setLed ( channel,  value + m_leds[channel].value, fade );
}


int plugin::countChannels() {
    return m_channels;
}

void plugin::setLed ( const QString& channel, int value, int fade ) {
    if ( !m_socket ) return;
    if ( !m_leds.contains(channel) ) return;
    ledchannel* l = &(m_leds[channel]);

    value = qBound ( 0, value, 255 );
    l->value = value;
    ledChanged ( channel, value );

    struct
    {
        uint8_t type;    // see above
        uint8_t channel; // if port: pin
        uint8_t value;
    } data;

    data.type = fade;
    data.channel = l->channel;
    data.value = value;

    m_socket->write ( (char*)&data, sizeof ( data ) );
}

void plugin::readyRead() {
    m_connectTimer.stop();
    while (m_socket->hasPendingDatagrams()) {
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size() );

        while ( bytes.size() > 6 ) {
            if (bytes.startsWith("stella") && bytes.size() >= 7+bytes[6])  {
                m_channels = bytes[6];
                clear();
                for (uint8_t c=0;c<m_channels;++c) {
                    const unsigned int value = (uint8_t)bytes[7+c];
                    m_leds[QString::number(c)] = ledchannel(c, value);
                    ledChanged(QString::number(c), value);
                }
                bytes = bytes.mid(7+m_channels);
            } else {
                qWarning() << pluginid() << "Failed to parse" << bytes << bytes.size() << 7+bytes[6];
                break;
            }
        } //while
    }
}

void plugin::connectToLeds ( const QString& host, int port ) {
    clear();
    m_sendPort = port;
    delete m_socket;
    m_socket = new QUdpSocket(this);
    connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_socket->connectToHost(QHostAddress(host),m_sendPort);
    m_connectTime=1000;
    m_connectTimer.start(m_connectTime);
}

void plugin::resendConnectSequence() {
    // request all channel values
    const unsigned char b[] = {255,0,0};
    m_socket->write ( (const char*)b, sizeof ( b ) );
    m_socket->flush();
    m_connectTime *= 2;
    if (m_connectTime>60000)
        m_connectTime=60000;
    m_connectTimer.start(m_connectTime);
}

