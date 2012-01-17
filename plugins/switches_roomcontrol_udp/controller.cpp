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

bool operator<(const Controller::ledid& src, const Controller::ledid& vgl) {
    return (src.port<vgl.port || (src.port==vgl.port && src.pin < vgl.pin));
}

Controller::Controller ( AbstractPlugin* plugin ) :m_plugin ( plugin ), m_socket ( 0 ) {
}

Controller::~Controller() {
    delete m_socket;
}

void Controller::setChannelName ( const Controller::ledid& channel, const QString& name ) {
    if ( !m_leds.contains(channel) ) return;
    m_leds[channel].name = name;
    emit ledChanged ( getStringFromPortPin(channel), name, -1 );

    QSettings settings;
    settings.beginGroup ( m_plugin->pluginid() );
    settings.beginGroup ( QLatin1String ( "channels" ) );
    settings.setValue ( QLatin1String ( "channel_name" ) + getStringFromPortPin(channel) , name );
}

bool Controller::getChannel ( const Controller::ledid& channel ) const {
    return m_leds.value ( channel ).value;
}

void Controller::toogleChannel ( const Controller::ledid& channel ) {
    if ( !m_leds.contains(channel) ) return;
    setChannel ( channel, !m_leds[channel].value );
}

int Controller::countChannels() {
    return m_leds.size();
}

QString Controller::getChannelName ( const Controller::ledid& channel ) {
    return m_leds.value ( channel ).name;
}

void Controller::setChannel ( const Controller::ledid& channel, bool value ) {
    if ( !m_socket ) return;
    if ( !m_leds.contains(channel) ) return;
    m_leds[channel].value = value;

    emit ledChanged ( getStringFromPortPin(channel), QString::null, value );

    struct udpio_packet
    {
        /* Port: 0=PORTA, ..., 3=PORTD, ...
        */
        uint8_t port;
        /* Pins: Every bit corresponds to one pin, where the least significant (right) bit
        means pin 0.
        */
        uint8_t pinmask;
        uint8_t mode;
    } data;

    data.port = channel.port;

    if (value) {
        data.mode = 1;
        data.pinmask = (1 << channel.pin);
    } else {
        data.mode = 0;
        data.pinmask = (1 << channel.pin);
    }

    m_socket->write ( (char*)&data, sizeof ( data ) );
}

void Controller::readyRead() {
    QSettings settings;
    settings.beginGroup ( m_plugin->pluginid() );
    settings.beginGroup ( QLatin1String ( "channels" ) );

    while (m_socket->hasPendingDatagrams()) {
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size() );

        while ( bytes.size() >= 7 ) {
            if (bytes.startsWith("pins") && bytes.size() >= 8)  {
                m_leds.clear();
                emit ledsCleared();

                for (int port=0;port<4;++port) {
                    for (int pin=0;pin<8;++pin) {
                        const bool value = bytes[4+port] & (1 << pin);
                        const QString name = settings.value(QLatin1String ( "channel_name" ) + getStringFromPortPin(ledid(port, pin)),tr("Channel Port %1, Pin %2").arg(port).arg(pin)).toString();
                        const ledid id = ledid(port, pin);
                        m_leds[id] = ledchannel(port, pin, value, name);
                        emit ledChanged(getStringFromPortPin(id), name, value);
                    }
                }
                bytes = bytes.mid(8);
            } else if (bytes.startsWith("pinc") && bytes.size() >= 6) {
                emit watchpinChanged(bytes[4], bytes[5]);
            } else {
                qWarning() << m_plugin->pluginid() << "Failed to parse" << bytes;
                break;
            }
        } //while
    }
}

void Controller::connectToLeds ( const QString& host, int port ) {
    m_sendPort = port;
    delete m_socket;
    m_socket = new QUdpSocket(this);
    connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_socket->connectToHost(QHostAddress(host),m_sendPort);

    // request all pin values
    char b[] = {0,0,3};
    m_socket->write ( b, sizeof ( b ) );
    m_socket->flush();
}

void Controller::registerPortObserver(unsigned char ioport, unsigned char pinmask) const {
    // request all pin values
    char b[] = {ioport,pinmask,4};
    m_socket->write ( b, sizeof ( b ) );
    m_socket->flush();
}

Controller::ledid Controller::getPortPinFromString(const QString& channel) const {
    Controller::ledid lid(-1,0);
    if (channel.size()<3) {
        qWarning() << m_plugin->pluginid() << "getPortPinFromChannel failed. Right syntax: e.g. A:0 or 2:7" << channel;
        return lid;
    }

    char port = channel[0].toAscii();
    lid.pin = channel[2].toAscii()-'0';

    switch (port) {
    case 'A':
    case '0':
        lid.port = 0;
        break;
    case 'B':
    case '1':
        lid.port = 1;
        break;
    case 'C':
    case '2':
        lid.port = 2;
        break;
    case 'D':
    case '3':
        lid.port = 3;
        break;
    case 'E':
    case '4':
        lid.port = 4;
        break;
    case 'F':
    case '5':
        lid.port = 5;
        break;
    default:
        lid.port = 250;
    }

    if (lid.port == 250 || lid.pin >= 8) {
        qWarning() << m_plugin->pluginid() << "getPortPinFromChannel failed. Port range (A-F) or Pin range (0-7) wrong!" << channel;
    }

    return lid;
}

QString Controller::getStringFromPortPin(const Controller::ledid& channel) const {
    return QString::number(channel.port) + QLatin1String(":") + QString::number(channel.pin);
}
