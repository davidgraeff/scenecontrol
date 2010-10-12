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

#include "ledcontroller.h"
#include <bitset>
#include "RoomControlServer.h"

#define _BV(bit) (1<<(bit))
#include <QSettings>
#include "stateTracker/curtainstatetracker.h"
#include "stateTracker/pinvaluestatetracker.h"
#include "stateTracker/pinnamestatetracker.h"
#include "stateTracker/channelvaluestatetracker.h"
#include "stateTracker/channelnamestatetracker.h"
#include "iocontroller.h"

LedController::LedController()
{
    m_udpSocket_lights = 0;
    m_sendTimer_lights.setInterval(20);
    m_sendTimer_lights.setSingleShot(true);
    connect(&m_sendTimer_lights,SIGNAL(timeout()), SLOT(sendTimeout_lights()));
    m_tryReconnectTimer_lights.setInterval ( 10000 );
    connect ( &m_tryReconnectTimer_lights,SIGNAL ( timeout() ),SLOT ( reconnect_lights() ) );
}

LedController::~LedController()
{
    qDeleteAll(m_channelvalues);
    qDeleteAll(m_channelnames);
    m_udpSocket_lights->disconnectFromHost();
}

void LedController::connectTo ( QHostAddress host, int udpport )
{
    m_udpport_lights = udpport;
    m_host_lights = host;
    if ( m_udpSocket_lights ) delete m_udpSocket_lights;
    m_udpSocket_lights = new QUdpSocket ( this );
    connect ( m_udpSocket_lights,SIGNAL ( connected() ),SLOT ( connected_lights() ) );
    connect ( m_udpSocket_lights,SIGNAL ( disconnected() ),SLOT ( disconnected_lights() ) );
    connect ( m_udpSocket_lights,SIGNAL ( readyRead() ),SLOT ( readyRead_lights() ) );
    connect ( m_udpSocket_lights,SIGNAL ( error ( QAbstractSocket::SocketError ) ),SLOT ( error_lights ( QAbstractSocket::SocketError ) ) );
    m_udpSocket_lights->connectToHost ( host, udpport );
}

void LedController::reconnect_lights()
{
    connectTo ( m_host_lights, m_udpport_lights );
}

void LedController::readyRead_lights()
{
    if ( ! m_udpSocket_lights->bytesAvailable() ) return;
    QByteArray buffer = m_udpSocket_lights->readAll();
    QSettings settings;
    while ( buffer.size() >1 )
    {
        if (!buffer.startsWith("stella"))
        {
            buffer.clear();
            break;
        }

        settings.beginGroup ( QLatin1String("channelnames") );
        udpstella_answer* ans = ( udpstella_answer* ) buffer.data();
        for ( int i=0;i<ans->channels;++i )
        {
            uint newvalue = ans->channel[i];
            if ( m_channelvalues.size() <i+1 )
            {
                const QString name = settings.value ( QLatin1String("channel")+QString::number( i ),
                                                      tr("Channel %1").arg( i ) ).toString();
                ChannelValueStateTracker* cv = new ChannelValueStateTracker();
                m_channelvalues.append(cv);
                cv->setChannel(i);
                cv->setValue(newvalue);
                cv->sync();
                ChannelNameStateTracker* cn = new ChannelNameStateTracker();
                m_channelnames.append(cn);
                cn->setChannel(i);
                cn->setValue(name);
                cn->sync();
            }
            else if ( m_channelvalues[i]->value() != newvalue )
            {
                m_channelvalues[i]->setValue(newvalue);
                m_channelvalues[i]->sync();
            }
        }
        buffer = buffer.mid ( ans->channels+7 );
    }
    emit dataAvailable();
}

void LedController::error_lights ( QAbstractSocket::SocketError err )
{
    qDebug() << __FUNCTION__ << __LINE__ << err;
}

void LedController::disconnected_lights()
{
    m_tryReconnectTimer_lights.start();
    m_sendTimer_lights.stop();
}

void LedController::connected_lights()
{
    if ( m_udpSocket_lights->isOpen() ) {
        m_tryReconnectTimer_lights.stop();
        udpstella_packet getch;
        getch.type = STELLA_GETALL;
        getch.channel = 0;
        getch.value = 0;
        m_queue_lights.insert ( -1, getch );
        m_sendTimer_lights.start();
    }
}

void LedController::sendTimeout_lights()
{
    QByteArray bytes;
    foreach ( const udpstella_packet d, m_queue_lights )
    {
        bytes.append ( ( const char* ) &d,sizeof ( d ) );
    }
    m_queue_lights.clear();
    m_udpSocket_lights->write( bytes );
}

void LedController::refresh()
{
    connected_lights();
}

QList<AbstractStateTracker*> LedController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    foreach (ChannelValueStateTracker* p, m_channelvalues) l.append(p);
    foreach (ChannelNameStateTracker* p, m_channelnames) l.append(p);
    return l;
}

void LedController::setChannelName ( uint channel, const QString& name )
{
    if ( channel>= ( unsigned int ) m_channelnames.size() ) return;
    m_channelnames[channel]->setValue(name);
    m_channelnames[channel]->sync();
    QSettings settings;
    settings.beginGroup ( QLatin1String("channelnames") );
    settings.setValue ( QLatin1String("channel")+QString::number ( channel ), name );
}

unsigned int LedController::getChannel(unsigned int channel) const
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return 300;
    return m_channelvalues.at(channel)->value();
}

void LedController::setChannel ( uint channel, uint value, uint fade )
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    value = qBound ( ( unsigned int ) 0, value, ( unsigned int ) 255 );
    m_channelvalues[channel]->setValue(value);
    m_channelvalues[channel]->sync();

    udpstella_packet setch;
    setch.type = fade;
    setch.channel = channel;
    setch.value = value;

    m_queue_lights.insert ( channel, setch );
    m_sendTimer_lights.start();
}

void LedController::inverseChannel(uint channel, uint fade)
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    const unsigned int newvalue = 255 - m_channelvalues[channel]->value();
    setChannel(channel, newvalue, fade);
}

void LedController::setChannelExponential ( uint channel, int multiplikator, uint fade )
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    unsigned int v = m_channelvalues[channel]->value();
    if (multiplikator>100) {
        if (v==0)
            v=1;
        else if (v==1)
            v=2;
        else
            v = (v * multiplikator)/100;
    } else {
        if (v==0 || v==1)
            v = 0;
        else
            v = (v * multiplikator)/100;
    }

    setChannel(channel, v, fade);
}

void LedController::setChannelRelative ( uint channel, int value, uint fade )
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    value += m_channelvalues[channel]->value();
    const unsigned int v = ( unsigned int ) qMin ( 0, value);
    setChannel ( channel, v, fade );
}


int LedController::countChannels()
{
    return m_channelvalues.size();
}
