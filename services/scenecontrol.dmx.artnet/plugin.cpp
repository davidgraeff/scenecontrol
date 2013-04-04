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
    
    if (AbstractPlugin::createInstance<plugin>(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 2;
    return app.exec();
}

plugin::~plugin() {
    delete m_socket;
}

void plugin::clear() {
    m_connectTimer.stop();
    m_leds.clear();
	
	changeProperty(SceneDocument::createModelReset("artnet.values", "channel").getData());
	
	SceneDocument target;
	target.setComponentID(m_pluginid);
	target.setInstanceID(m_instanceid);
	SceneDocument doc;
    doc.setMethod("clear");
	doc.setData("target",target.getData());	
	doc.setComponentID(QLatin1String("scenecontrol.leds"));
	doc.setInstanceID(QLatin1String("null"));
	callRemoteComponent(doc.getData());
}

void plugin::initialize() {
	connect(&m_connectTimer, SIGNAL(timeout()), SLOT(resendConnectSequence()));
	m_connectTimer.setSingleShot(true);
	
	clear();
	delete m_socket;
	m_socket = new QUdpSocket(this);
	connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
	m_socket->bind(6454);
	m_connectTime=1000;
	resendConnectSequence();
}

void plugin::instanceConfiguration(const QVariantMap& data) {
	Q_UNUSED(data);

}

bool plugin::isLedValue( const QString& channel, int lower, int upper ) {
    const int v = getLed ( channel );
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

void plugin::requestProperties() {
	changeProperty(SceneDocument::createModelReset("artnet.values", "channel").getData(), m_lastsessionid);

    QMap<QString, plugin::ledchannel>::iterator i = m_leds.begin();
    for (;i!=m_leds.end();++i) {
        {
			SceneDocument sc = SceneDocument::createModelChangeItem("artnet.values");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            changeProperty(sc.getData(), m_lastsessionid);
        }
    }
}

void plugin::ledChanged(QString channel, int value) {
	SceneDocument sc = SceneDocument::createModelChangeItem("artnet.values");
    sc.setData("channel", channel);
    if (value != -1) sc.setData("value", value);
    changeProperty(sc.getData());

	SceneDocument target;
	target.setComponentID(m_pluginid);
	target.setInstanceID(m_instanceid);
	SceneDocument doc;
	doc.setMethod("subpluginChange");
	doc.setData("target",target.getData());
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

	char mem[sizeof(artnet_dmx)+m_leds.size()];
	artnet_dmx* out = (artnet_dmx*)&mem;
	out->opcode = OP_OUTPUT;
	out->universe = l->universe;
	out->lengthHi = 0;
	out->length = m_leds.size();
	strcpy(out->id, artnet_ID);
	
	for (auto i=m_leds.begin();i!=m_leds.end();++i) {
		const ledchannel& cu = *i;
		if (cu.channel<m_leds.size()) {
			out->dataStart[cu.channel] = cu.value;
		}
	}

	qDebug() << "write" << l->remote << sizeof ( mem );
	m_socket->writeDatagram ( mem, sizeof ( mem ), l->remote, 6454 );
}

void plugin::readyRead() {
    m_connectTimer.stop();
    while (m_socket->hasPendingDatagrams()) {
		QHostAddress remote;
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size(), &remote );
		
		const artnet_header *header= (const artnet_header *) bytes.constData();
		
		/* check the id */
		if (strncmp((char *) header->id, artnet_ID, 8))
		{
			qWarning("Wrong ArtNet header, discarded\r\n");
			continue;
		}
		
		switch (header->opcode)
		{
			case OP_POLL:
				qDebug() << "Received artnet poll packet!" << remote.toString();
				break;
			case OP_POLLREPLY:{
				const artnet_pollreply *pollreply = (const artnet_pollreply *) header;
				
				qDebug() << "Received artnet poll reply packet!" << pollreply->shortName << pollreply->longName;
				break;
			} case OP_OUTPUT: {
				const artnet_dmx *dmx = (const artnet_dmx *) header;
				qDebug() << "Received artnet data packet!" << dmx->universe << dmx->length;
				m_channels = dmx->length;
				clear();
				for (uint8_t c=0;c<m_channels;++c) {
					const unsigned int value = (uint8_t)dmx->dataStart[c];
					m_leds[QString::number(c)] = ledchannel(c, value,remote, dmx->universe);
					ledChanged(QString::number(c), value);
				}
				break;
			}
			case OP_ADDRESS:
			case OP_IPPROG:
				break;
			default:
				qDebug("Received an invalid artnet packet!\r\n");
				break;
		}
    }
}

void plugin::resendConnectSequence() {
    // request all nodes
	artnet_poll p;
	strcpy(p.id, artnet_ID);
	p.opcode = OP_POLL;
	p.versionH = 0;
	p.version = PROTOCOL_VERSION;
	p.talkToMe = 0 | (1 << 2) | ( 1 << 1);
	p.priority = 0;
	
	m_socket->writeDatagram ( (const char*)&p, sizeof ( p ), QHostAddress::Broadcast, 6454);
    m_socket->flush();
	
    m_connectTime *= 2;
    if (m_connectTime>60000)
        m_connectTime=60000;
    m_connectTimer.start(m_connectTime);
}
