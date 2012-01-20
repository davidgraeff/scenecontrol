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
#include <shared/qxtserialdevice/qxtserialdevice.h>
#include <shared/abstractplugin.h>

Controller::Controller ( AbstractPlugin* plugin ) : m_curtain_max ( 0 ), m_curtain_value ( 0 ), m_plugin ( plugin ), m_channels ( 0 ), m_bufferpos ( 0 ), m_readState ( ReadOK ), m_serial ( 0 ) {
    m_panicTimeoutAck = false;
    connect(&m_moodlightTimer, SIGNAL(timeout()),SLOT(moodlightTimeout()));
    m_moodlightTimer.setInterval(5000);
    srand(100);
}

Controller::~Controller() {
    delete m_serial;
}

void Controller::readyRead() {
    m_connected = true;
    QByteArray bytes;
    bytes.resize ( m_serial->bytesAvailable() );
    m_serial->read ( bytes.data(), bytes.size() );
    m_buffer.append ( bytes );
    m_readState = ReadOK;
    while ( m_buffer.size() ) {
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
            switch ( m_buffer[0] ) {
            case 'S': //sensors
                if ( m_buffer.size() <2 ) break;
                parseSensors ( m_buffer[1] );
                break;
            case 'M': //curtain motor
                if ( m_buffer.size() <3 ) break;
                parseCurtain ( m_buffer[1], m_buffer[2] );
                break;
            case 'L': //leds
                if ( m_buffer.size() <2 ) break;
                m_channels = m_buffer[1];
                parseLeds ( m_buffer.mid ( 2,m_channels ) );
                break;
            case 'I': //init
                if ( m_buffer.size() <2 ) break;
                parseInit ( m_buffer[1] );
                break;
            case 'O': //panic timeout ack
                m_panicTimeoutAck = true;
                break;
            default:
                qWarning()<< m_plugin->pluginid() << "command unknown: (Command, BufferSize)" << m_buffer[0] << m_buffer.size();
                m_buffer.clear();
                m_bufferpos = 0;
            }
            m_readState = ReadOK;
        }
    } //while
}

void Controller::parseCurtain ( unsigned char current, unsigned char max ) {
    m_curtain_value = current;
    m_curtain_max = max;
    emit curtainChanged ( current, max );
}

void Controller::parseInit ( unsigned char protocolversion ) {
    qDebug() <<m_plugin->pluginid() << "Firmware Protocol:" << protocolversion;
}

void Controller::parseSensors ( unsigned char s1 ) {
    qDebug() <<m_plugin->pluginid() << "Sensors:" << (int)s1;
}

void Controller::parseLeds ( const QByteArray& data ) {
    if ( data.isEmpty() || data.size() != m_channels ) {
        qWarning() <<m_plugin->pluginid() <<__FUNCTION__<<"size missmatch:"<< m_channels <<data.size();
        return;
    }
    QSettings settings;
    settings.beginGroup ( m_plugin->pluginid() );
    settings.beginGroup ( QLatin1String ( "channels" ) );
    // clear old
    m_leds.clear();
    emit ledsCleared();
    // set new
    qDebug() <<m_plugin->pluginid() << "LED Channels:" << m_channels;
    for ( int i=0;i<m_channels;++i ) {
        const QString channelid = QString::number(i);
        ledchannel l;
        l.value = ( uint8_t ) data[i];
        l.name = settings.value ( QLatin1String ( "channel_name" ) +QString::number ( i ),
                                  tr ( "Channel %1" ).arg ( i ) ).toString();
        l.moodlight =  settings.value ( QLatin1String ( "channel_moodlight" ) +QString::number ( i )).toBool();
        if (l.moodlight) m_moodlightTimer.start();

        m_leds[channelid] = l;
        emit ledChanged ( channelid, l.name, l.value );
    }
    emit dataLoadingComplete();
}

void Controller::setCurtain ( unsigned int position ) {
    if ( !m_serial ) return;
    m_curtain_value = position;
    emit curtainChanged ( m_curtain_value, m_curtain_max );
    const char t1[] = {0xdf, position};
    m_serial->write ( t1, sizeof ( t1 ) );
}

int Controller::getCurtain() {
    return m_curtain_value;
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

void Controller::setChannel ( const QString& channel, uint value, uint fade ) {
    if ( !m_serial ) return;
    if ( !m_leds.contains(channel) ) return;
    value = qBound ( ( unsigned int ) 0, value, ( unsigned int ) 255 );
    m_leds[channel].value = value;
    emit ledChanged ( channel, QString::null, value );

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
    const char t1[] = {cfade, channel.toUInt(), value};
    m_serial->write ( t1, sizeof ( t1 ) );
}

void Controller::moodlight(const QString& channel, bool moodlight) {
    if ( !m_leds.contains(channel) ) return;
    m_leds[channel].moodlight = moodlight;
    QSettings settings;
    settings.beginGroup ( m_plugin->pluginid() );
    settings.beginGroup ( QLatin1String ( "channels" ) );
    settings.setValue ( QLatin1String ( "channel_moodlight" ) +channel , moodlight );
    if (moodlight) {
        m_moodlightTimer.start();
        moodlightTimeout();
    }
}

void Controller::inverseChannel ( const QString& channel, uint fade ) {
    if ( !m_leds.contains(channel) ) return;
    const unsigned int newvalue = 255 - m_leds[channel].value;
    setChannel ( channel, newvalue, fade );
}

void Controller::setChannelExponential ( const QString& channel, int multiplikator, uint fade ) {
    if ( !m_leds.contains(channel) ) return;
    unsigned int v = m_leds[channel].value;
    if ( multiplikator>100 ) {
        if ( v==0 )
            v=1;
        else if ( v==1 )
            v=2;
        else
            v = ( v * multiplikator ) /100;
    } else {
        if ( v==0 || v==1 )
            v = 0;
        else
            v = ( v * multiplikator ) /100;
    }

    setChannel ( channel, v, fade );
}

void Controller::setChannelRelative ( const QString& channel, int value, uint fade ) {
    if (! m_leds.contains(channel) ) return;
    value += m_leds[channel].value;
    const unsigned int v = ( unsigned int ) qMin ( 0, value );
    setChannel ( channel, v, fade );
}


int Controller::countChannels() {
    return m_channels;
}

QString Controller::getChannelName ( const QString& channel ) {
    return m_leds.value ( channel ).name;
}

void Controller::panicTimeout() {
    if ( !m_serial ) return;
    const char t1[] = {0x00};
    if ( !m_serial->isOpen() || m_serial->write ( t1, sizeof ( t1 ) ) == -1 ) {
        if (m_connected) {
            m_connected = false;
            qWarning() << m_plugin->pluginid() << "Failed to reset panic counter. Try reconnection";
        }
        m_serial->close();
        const char t1[] = {0xef};
        if ( !m_serial->open ( QIODevice::ReadWrite ) || !m_serial->write ( t1,  sizeof ( t1 ) ) ) {
            qWarning() << m_plugin->pluginid() << "rs232 init fehler";
        }
    }

    if (!m_panicTimeoutAck) {
        qWarning() << m_plugin->pluginid() << "No panic timeout ack!";
    }
    m_panicTimeoutAck = false;
}
void Controller::connectToLeds ( const QString& device ) {
    // disable old
    m_panicTimer.stop();
    delete m_serial;
    m_serial = 0;
    m_connected = false;
    // create new

    // Open device and ask for pins
    if ( !QFile::exists ( device ) ) {
        qWarning() << m_plugin->pluginid() << "device not found"<<device;
        return;
    }

    m_serial = new QxtSerialDevice ( device,QxtSerialDevice::EventDriven );
    m_serial->setBaudRate ( BAUD115200 );
    m_serial->setFlowControl ( FLOW_OFF );
    m_serial->setParity ( PAR_NONE );
    m_serial->setDataBits ( DATA_8 );
    m_serial->setStopBits ( STOP_1 );
    connect ( m_serial, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );

    const char t1[] = {0xef};
    if ( !m_serial->open ( QIODevice::ReadWrite ) || !m_serial->write ( t1, sizeof ( t1 ) ) ) {
        qWarning() << m_plugin->pluginid() << "rs232 error:" << m_serial->errorString();
    } else {
        qDebug() << m_plugin->pluginid() << "connected to"<<device;
    }

    // panic counter
    m_panicTimer.setInterval ( 50000 );
    connect ( &m_panicTimer,SIGNAL ( timeout() ),SLOT ( panicTimeout() ) );
    m_panicTimer.start();
}

void Controller::moodlightTimeout() {
    QMap<QString,ledchannel>::iterator i = m_leds.begin();
    int c = 0;
    for (;i != m_leds.end();++i) {
        if (!i.value().moodlight) continue;
        ++c;
        if (rand()/RAND_MAX >0.5) continue;
        setChannel(i.key(),rand()%255,STELLA_SET_FADE);
    }
    if (!c) m_moodlightTimer.stop();
}
