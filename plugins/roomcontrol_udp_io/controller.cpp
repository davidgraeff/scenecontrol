/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "controller.h"
#include <QSettings>
#include <qfile.h>
#include <QDebug>
#include <shared/qextserialport/qextserialport.h>
#include <shared/abstractplugin.h>

Controller::Controller ( AbstractPlugin* plugin ) :m_plugin ( plugin ), m_channels ( 0 ), m_socket ( 0 ) {
}

Controller::~Controller() {
    delete m_socket;
}

void Controller::setChannelName ( const QString& channel, const QString& name ) {
    if ( !m_leds.contains(channel) ) return;
    m_leds[channel].name = name;
    emit ledChanged ( channel, name, -1 );

    QSettings settings;
    settings.beginGroup ( m_plugin->pluginid() );
    settings.beginGroup ( QLatin1String ( "channels" ) );
    settings.setValue ( QLatin1String ( "channel_name" ) + channel , name );
}

unsigned int Controller::getChannel ( const QString& channel ) const {
    return m_leds.value ( channel ).value;
}

void Controller::toogleChannel ( const QString& channel ) {
    if ( !m_leds.contains(channel) ) return;
    setChannel ( channel, !m_leds[channel].value );
}

int Controller::countChannels() {
    return m_channels;
}

QString Controller::getChannelName ( const QString& channel ) {
    return m_leds.value ( channel ).name;
}

void Controller::setChannel ( const QString& channel, bool value ) {
    if ( !m_socket ) return;
    if ( !m_leds.contains(channel) ) return;
    ledchannel* l = &(m_leds[channel]);

    l->value = value;
    emit ledChanged ( channel, QString::null, value );

    struct
    {
        /* Port, where 0=PORTA, ..., 3=PORTD
           255=get all pins of all ports
        */
        uint8_t port;
        /* Pins: Function depends on "nstate". Every bit
           corresponds to one pin, where the most significant bit
           means pin 0 of the port selected above.
           if nstate is 0: (disable)
             Disables all pins, where the corresponding bit of "pins"
             is set to 0.
           if nstate is 1: (enable)
             Enables all pins, where the corresponding bit of "pins"
             is set to 1.
           if nstate is 2: (set)
             Set the port to the value of "pins".
        */
        uint8_t pins;
        uint8_t nstate;
    } data;

	if (channel.size()<3){
		qWarning() << m_plugin->pluginid() << "SetChannel failed. Right syntax: e.g. A:0 or 2:7" << channel;
		return;
	}
	
	char port = channel[0].toAscii();
	int pin = channel[2].toAscii()-'0';
	
	switch (port) {
		case 'A':
		case '0':
			data.port = 0;
			break;
		case 'B':
		case '1':
			data.port = 1;
			break;
		case 'C':
		case '2':
			data.port = 2;
			break;
		case 'D':
		case '3':
			data.port = 3;
			break;
		case 'E':
		case '4':
			data.port = 4;
			break;
		case 'F':
		case '5':
			data.port = 5;
			break;
		default:
			data.port = 250;
	}
	
	if (data.port == 250 || pin<0 || pin >= 8){
		qWarning() << m_plugin->pluginid() << "SetChannel failed. Port range (A-F) or Pin range (0-7) wrong!" << channel;
		return;
	}
	
    if (value) {
		data.nstate = 1;
		data.pins = (1 << pin);
	} else {
		data.nstate = 0;
		data.pins = ~(1 << pin);
	}
	
    m_socket->write ( (char*)&data, sizeof ( data ) );
}

void Controller::readyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size() );

        while ( bytes.size() >= 7 ) {
            if (bytes.startsWith("Stella") && bytes.size() >= 7+bytes[6])  {
                m_channels = bytes[6];
                m_leds.clear();
                for (uint8_t c=0;c<m_channels;++c) {
                    m_leds[QString::number(c)] = ledchannel(c, bytes[7+c]);
                }
                bytes = bytes.mid(7+m_channels);
                emit ledsCleared();
            } else {
                qWarning() << m_plugin->pluginid() << "Failed to parse" << bytes;
                break;
            }
        } //while
    }
}

void Controller::connectToLeds ( const QString& server ) {
    const int v = server.indexOf(QLatin1Char(':'));
    if (v==-1) {
        qWarning() << m_plugin->pluginid() << "Configuration wrong (server:port)" << server;
        return;
    }
    m_sendPort = server.mid(v+1).toInt();
    delete m_socket;
    m_socket = new QUdpSocket(this);
    connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_socket->bind(QHostAddress(server.mid(0,v)),m_sendPort);
}
