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
    connect(&m_moodlightTimer, SIGNAL(timeout()),SLOT(moodlightTimeout()));
    m_moodlightTimer.setInterval(5000);
    srand(100);
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
            v = ( v * multiplikator ) /100+1;
    } else {
        if ( v<2 )
            v = 0;
        else
            v = ( v * multiplikator ) /100-1;
    }

    setChannel ( channel, v, fade );
}

void Controller::setChannelRelative ( const QString& channel, int value, uint fade ) {
    if (! m_leds.contains(channel) ) return;
    setChannel ( channel,  value + m_leds[channel].value, fade );
}


int Controller::countChannels() {
    return m_channels;
}

QString Controller::getChannelName ( const QString& channel ) {
    return m_leds.value ( channel ).name;
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



void Controller::setChannel ( const QString& channel, int value, uint fade ) {
    if ( !m_socket ) return;
    if ( !m_leds.contains(channel) ) return;
    ledchannel* l = &(m_leds[channel]);

    value = qBound ( 0, value, 255 );
    l->value = value;
    emit ledChanged ( channel, QString::null, value );

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

void Controller::readyRead() {
    QSettings settings;
    settings.beginGroup ( m_plugin->pluginid() );
    settings.beginGroup ( QLatin1String ( "channels" ) );

    while (m_socket->hasPendingDatagrams()) {
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size() );

        while ( bytes.size() >= 7 ) {
            if (bytes.startsWith("stella") && bytes.size() >= 7+bytes[6])  {
                m_channels = bytes[6];
                m_leds.clear();
                emit ledsCleared();
                for (uint8_t c=0;c<m_channels;++c) {
                    const unsigned int value = (uint8_t)bytes[7+c];
                    const QString name = settings.value(QLatin1String ( "channel_name" ) + QString::number(c),tr("Channel %1").arg(c)).toString();
                    m_leds[QString::number(c)] = ledchannel(c, value, name);
                    emit ledChanged(QString::number(c), name, value);
                }
                bytes = bytes.mid(7+m_channels);
            } else {
                qWarning() << m_plugin->pluginid() << "Failed to parse" << bytes << bytes.size() << 7+bytes[6];
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

    // request all channel values
    char b[] = {255,0,0};
    m_socket->write ( b, sizeof ( b ) );
    m_socket->flush();
}
