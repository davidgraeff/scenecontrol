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
#include "rs232.h"
#include <QCoreApplication>
#include <qfile.h>

rs232leds::rs232leds() :m_curtain_max ( 0 ), m_curtain_value ( 0 ) , m_bufferpos ( 0 ), m_readState ( ReadOK ), m_serial ( 0 ) {
    m_panicTimeoutAck = false;
}

rs232leds::~rs232leds() {
	disconnectLeds();
}

void rs232leds::disconnectLeds() {
    m_leds.clear();
    m_panicTimer.stop();
	if (m_serial )m_serial->close();
    delete m_serial;
    m_serial = 0;
    m_connected = false;
}

void rs232leds::readyRead() {
	{
		QByteArray bytes;
		bytes.resize ( m_serial->bytesAvailable() );
		m_serial->read ( bytes.data(), bytes.size() );
		m_buffer.append ( bytes );
	}

	if ( m_buffer.isEmpty() )
		return;

	if ( m_readState==ReadOK ) {
		for ( int i=m_bufferpos;i<m_buffer.size();++i ) {
			if ( m_buffer.size() <=i ) break;
			if ( m_buffer[i] == 'O' && m_buffer[i+1] == 'K' ) {
				m_readState = ReadEnd;
				m_buffer.remove ( 0,i+2 );
				m_bufferpos = 0;
				break;
			}
		}
		if ( m_readState==ReadOK ) {
			m_buffer.clear();
			m_bufferpos = 0;
			return;
		}
	}
	if ( m_readState == ReadEnd && m_buffer.size() >1 ) {
		bool finished = false;
// 		qDebug() << "Read" << m_buffer[0];
		switch ( m_buffer[0] ) {
		case 'S': //sensors
			if ( m_buffer.size() <2 ) break;
			parseSensors ( m_buffer[1] );
			finished = true;
			break;
		case 'M': //curtain motor
			if ( m_buffer.size() <3 ) break;
			parseCurtain ( m_buffer[1], m_buffer[2] );
			finished = true;
			break;
		case 'L': //leds
			if ( m_buffer.size() <2 || m_buffer.size() <m_buffer[1]+2 ) break;
			parseLeds ( m_buffer.mid ( 2, m_buffer[1] ), m_buffer[1] );
			finished = true;
			break;
		case 'I': //init
			if ( m_buffer.size() <2 ) break;
			parseInit ( m_buffer[1] );
			finished = true;
			break;
		case 'A': //panic timeout ack
			m_panicTimeoutAck = true;
			finished = true;
			break;
		default:
			qWarning()<< "Leds.RS232" << "command unknown: (Command, BufferSize)" << m_buffer[0] << m_buffer.size();
		}
		// only if all bytes for the last state have been received change the state to check for new packets
		if (finished)
			m_readState = ReadOK;
	}
}

void rs232leds::parseCurtain ( unsigned char current, unsigned char max ) {
    m_curtain_value = current;
    m_curtain_max = max;
    curtainChanged ( current, max );
}

void rs232leds::parseInit ( unsigned char protocolversion ) {
	emit connectedToLeds(protocolversion);
//     qDebug() <<"Leds.RS232" << "Firmware Protocol:" << protocolversion;
}

void rs232leds::parseSensors ( unsigned char /*s1*/ ) {
//     qDebug() <<"Leds.RS232" << "Sensors:" << (int)s1;
}

void rs232leds::parseLeds ( const QByteArray& data, int channels ) {
	if ( data.isEmpty() || data.size() != channels ) {
		qWarning() <<"Leds.RS232" <<__FUNCTION__<<"size missmatch:"<< channels <<data.size();
        return;
    }
    m_leds.clear();
    // set new
	qDebug() <<"Leds.RS232" << "LED Channels:" << channels;
	for ( int i=0;i<channels;++i ) {
        const QString channelid = QString::number(i);
        ledchannel l;
        l.value = ( uint8_t ) data[i];

        m_leds[channelid] = l;
        ledChanged ( channelid, l.value );
    }
}

void rs232leds::setCurtain ( unsigned int position ) {
    if ( !m_serial ) return;
    m_curtain_value = position;
	const unsigned char t1[] = {0xff, 0xff, 0xdf, static_cast<unsigned char>(position)};
	if (m_serial->write ( (const char*)t1, sizeof ( t1 ) ) == sizeof ( t1 )) {
		emit curtainChanged ( m_curtain_value, m_curtain_max );
	} else {
		qWarning()<< "Leds.RS232" << "failed to set curtain value";
		m_serial->close();
	}
}

int rs232leds::getCurtain() {
    return m_curtain_value;
}

int rs232leds::getLed( const QString& channel ) const {
    return m_leds.value ( channel ).value;
}

void rs232leds::setLed ( const QString& channel, int value, int fade ) {
    if ( !m_serial ) return;
    if ( !m_leds.contains(channel) ) return;
    value = qBound ( 0, value, 255 );
    m_leds[channel].value = value;
    ledChanged ( channel, value );

    unsigned char cfade=0;
    switch ( fade ) {
    case STELLA_SET_IMMEDIATELY:
        cfade = 0xcf;
        break;
    case STELLA_SET_FADE:
        cfade = 0xbf;
        break;
    case STELLA_SET_FLASHY:
        cfade = 0xaf;
        break;
    case STELLA_SET_IMMEDIATELY_RELATIVE:
        cfade = 0x9f;
        break;
    default:
        break;
    };
	const unsigned char t1[] = {0xff, 0xff, cfade, static_cast<unsigned char>(channel.toUInt()), static_cast<unsigned char>(value)};
	if (m_serial->write ( (const char*)t1, sizeof ( t1 ) ) != sizeof ( t1 )) {
		qWarning()<< "Leds.RS232" << "failed to set led value";
		m_serial->close();
	}
}

int rs232leds::countLeds() {
    return m_leds.size();
}

void rs232leds::panicTimeout() {
    if ( !m_serial ) return;
	
	if (!m_panicTimeoutAck) {
		qWarning() << "Leds.RS232" << "No panic timeout ack!";
	}
	m_panicTimeoutAck = false;
	
	const unsigned char t1[] = {0xff, 0xff, 0x00};
	if (m_serial->write ( (const char*)t1, sizeof ( t1 ) ) != sizeof ( t1 ) ) {
		m_serial->close();
	}
	
	if (!m_serial->isOpen()) {
		qWarning() << "Leds.RS232" << "Failed to reset panic counter. Try reconnection";
		m_panicTimer.stop();
		QString devicename = m_serial->deviceName();
		connectToLeds(devicename);
	}
}

void rs232leds::connectToLeds ( const QString& device ) {
    disconnectLeds();

    if (device.isEmpty())
        return;

    // Open device and ask for pins
    if ( !QFile::exists ( device ) ) {
        qWarning() << "Leds.RS232" << "device not found"<<device;
        return;
    }

    m_serial = new QxtSerialDevice ( device );
    m_serial->setBaud ( QxtSerialDevice::Baud115200 );
    m_serial->setPortSettings ( QxtSerialDevice::FlowOff | QxtSerialDevice::ParityNone
                                | QxtSerialDevice::Stop1 | QxtSerialDevice::Bit8);
    connect ( m_serial, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );

	const unsigned char t1[] = {0xff, 0xff, 0xef};
	if ( !m_serial->open ( QIODevice::ReadWrite ) || m_serial->write ( (const char*)t1,  sizeof ( t1 ) ) != sizeof ( t1 ) ) {
        qWarning() << "Leds.RS232" << "rs232 error:" << m_serial->errorString();
    } else {
        qDebug() << "Leds.RS232" << "connected to"<<device;
    }

    // panic counter
    m_panicTimer.setInterval ( 40000 );
    connect ( &m_panicTimer,SIGNAL ( timeout() ),SLOT ( panicTimeout() ) );
    m_panicTimer.start();
}

