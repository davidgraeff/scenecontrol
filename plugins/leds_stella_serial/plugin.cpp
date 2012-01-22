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
#include <qfile.h>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() : AbstractPlugin(this), m_curtain_max ( 0 ), m_curtain_value ( 0 ) , m_channels ( 0 ), m_bufferpos ( 0 ), m_readState ( ReadOK ), m_serial ( 0 ) {
    m_panicTimeoutAck = false;
}

plugin::~plugin() {
    delete m_serial;
}

void plugin::clear() {
    m_leds.clear();
    changeProperty(ServiceData::createModelReset("roomcontrol.leds", "channel").getData());
    m_panicTimer.stop();
    delete m_serial;
    m_serial = 0;
    m_connected = false;
}
void plugin::initialize() {
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("serialport")))
        connectToLeds ( data[QLatin1String("serialport")].toString() );
}

bool plugin::isValue( const QString& channel, int lower, int upper ) {
    const int v = getChannel ( channel );
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

bool plugin::isCurtainInPosition( int lower, int upper ) {
    const int v = getCurtain();
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

void plugin::requestProperties(int sessionid) {
    changeProperty(ServiceData::createModelReset("roomcontrol.leds", "channel").getData(), sessionid);
    QMap<QString, plugin::ledchannel>::iterator i = m_leds.begin();
    for (;i!=m_leds.end();++i) {
        {
            ServiceData sc = ServiceData::createModelChangeItem("roomcontrol.leds");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            changeProperty(sc.getData(), sessionid);
        }
    }
    {
        ServiceData sc = ServiceData::createNotification("curtain.state");
        sc.setData("value", m_curtain_value);
        sc.setData("max", m_curtain_max);
        changeProperty(sc.getData(), sessionid);
    }
}

void plugin::curtainChanged(int current, int max) {
    ServiceData sc = ServiceData::createNotification("curtain.state");
    sc.setData("value", current);
    sc.setData("max", max);
    changeProperty(sc.getData());
}

void plugin::ledChanged(QString channel, int value) {
    ServiceData sc = ServiceData::createModelChangeItem("roomcontrol.leds");
    sc.setData("channel", channel);
    if (value != -1) sc.setData("value", value);
    changeProperty(sc.getData());
}

void plugin::readyRead() {
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
                qWarning()<< pluginid() << "command unknown: (Command, BufferSize)" << m_buffer[0] << m_buffer.size();
                m_buffer.clear();
                m_bufferpos = 0;
            }
            m_readState = ReadOK;
        }
    } //while
}

void plugin::parseCurtain ( unsigned char current, unsigned char max ) {
    m_curtain_value = current;
    m_curtain_max = max;
    emit curtainChanged ( current, max );
}

void plugin::parseInit ( unsigned char protocolversion ) {
    qDebug() <<pluginid() << "Firmware Protocol:" << protocolversion;
}

void plugin::parseSensors ( unsigned char s1 ) {
    qDebug() <<pluginid() << "Sensors:" << (int)s1;
}

void plugin::parseLeds ( const QByteArray& data ) {
    if ( data.isEmpty() || data.size() != m_channels ) {
        qWarning() <<pluginid() <<__FUNCTION__<<"size missmatch:"<< m_channels <<data.size();
        return;
    }
    clear();
    // set new
    qDebug() <<pluginid() << "LED Channels:" << m_channels;
    for ( int i=0;i<m_channels;++i ) {
        const QString channelid = QString::number(i);
        ledchannel l;
        l.value = ( uint8_t ) data[i];

        m_leds[channelid] = l;
        emit ledChanged ( channelid, l.value );
    }
}

void plugin::setCurtain ( unsigned int position ) {
    if ( !m_serial ) return;
    m_curtain_value = position;
    emit curtainChanged ( m_curtain_value, m_curtain_max );
    const char t1[] = {0xdf, position};
    m_serial->write ( t1, sizeof ( t1 ) );
}

int plugin::getCurtain() {
    return m_curtain_value;
}

unsigned int plugin::getChannel ( const QString& channel ) const {
    return m_leds.value ( channel ).value;
}

void plugin::setChannel ( const QString& channel, uint value, uint fade ) {
    if ( !m_serial ) return;
    if ( !m_leds.contains(channel) ) return;
    value = qBound ( ( unsigned int ) 0, value, ( unsigned int ) 255 );
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
    const char t1[] = {cfade, channel.toUInt(), value};
    m_serial->write ( t1, sizeof ( t1 ) );
}

void plugin::inverseChannel ( const QString& channel, uint fade ) {
    if ( !m_leds.contains(channel) ) return;
    const unsigned int newvalue = 255 - m_leds[channel].value;
    setChannel ( channel, newvalue, fade );
}

void plugin::setChannelExponential ( const QString& channel, int multiplikator, uint fade ) {
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

void plugin::setChannelRelative ( const QString& channel, int value, uint fade ) {
    if (! m_leds.contains(channel) )
        return;
    value += m_leds[channel].value;
    const unsigned int v = ( unsigned int ) qMin ( 0, value );
    setChannel ( channel, v, fade );
}

int plugin::countChannels() {
    return m_channels;
}

void plugin::panicTimeout() {
    if ( !m_serial ) return;
    const char t1[] = {0x00};
    if ( !m_serial->isOpen() || m_serial->write ( t1, sizeof ( t1 ) ) == -1 ) {
        if (m_connected) {
            m_connected = false;
            qWarning() << pluginid() << "Failed to reset panic counter. Try reconnection";
        }
        m_serial->close();
        const char t1[] = {0xef};
        if ( !m_serial->open ( QIODevice::ReadWrite ) || !m_serial->write ( t1,  sizeof ( t1 ) ) ) {
            qWarning() << pluginid() << "rs232 init fehler";
        }
    }

    if (!m_panicTimeoutAck) {
        qWarning() << pluginid() << "No panic timeout ack!";
    }
    m_panicTimeoutAck = false;
}

void plugin::connectToLeds ( const QString& device ) {
    clear();

    // Open device and ask for pins
    if ( !QFile::exists ( device ) ) {
        qWarning() << pluginid() << "device not found"<<device;
        return;
    }

    m_serial = new QxtSerialDevice ( device );
    m_serial->setBaud ( QxtSerialDevice::Baud115200 );
    m_serial->setPortSettings ( QxtSerialDevice::FlowOff | QxtSerialDevice::ParityNone
                                | QxtSerialDevice::Stop1 | QxtSerialDevice::Bit8);
    connect ( m_serial, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );

    const char t1[] = {0xef};
    if ( !m_serial->open ( QIODevice::ReadWrite ) || !m_serial->write ( t1, sizeof ( t1 ) ) ) {
        qWarning() << pluginid() << "rs232 error:" << m_serial->errorString();
    } else {
        qDebug() << pluginid() << "connected to"<<device;
    }

    // panic counter
    m_panicTimer.setInterval ( 50000 );
    connect ( &m_panicTimer,SIGNAL ( timeout() ),SLOT ( panicTimeout() ) );
    m_panicTimer.start();
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}

