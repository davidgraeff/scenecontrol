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
#include <bitset>

#define _BV(bit) (1<<(bit))
#include <QSettings>
#include "statetracker/curtainstatetracker.h"
#include "statetracker/ledvaluestatetracker.h"
#include "statetracker/lednamestatetracker.h"

Controller::Controller()
{
    m_udpSocket_lights = 0;
    m_sendTimer_lights.setInterval(20);
    m_sendTimer_lights.setSingleShot(true);
    connect(&m_sendTimer_lights,SIGNAL(timeout()), SLOT(sendTimeout_lights()));
    m_tryReconnectTimer_lights.setInterval ( 10000 );
    connect ( &m_tryReconnectTimer_lights,SIGNAL ( timeout() ),SLOT ( reconnect_lights() ) );

    m_curtainStateTracker = new CurtainStateTracker();

    m_udpSocket_curtain = 0;
    m_tryReconnectTimer_curtain.setInterval ( 10000 );
    connect ( &m_tryReconnectTimer_curtain,SIGNAL ( timeout() ),SLOT ( reconnect_curtain() ) );

}

Controller::~Controller()
{
    qDeleteAll(m_channelvalues);
    qDeleteAll(m_channelnames);
    m_udpSocket_lights->disconnectFromHost();
    delete m_curtainStateTracker;
    m_udpSocket_curtain->disconnectFromHost();
}

void Controller::reconnect_curtain()
{
    connectTo ( m_host, 0, m_udpport_curtain );
}

void Controller::readyRead_curtain()
{
    if ( ! m_udpSocket_curtain->bytesAvailable() ) return;
    QByteArray buffer = m_udpSocket_curtain->readAll();
    QSettings settings;
    while ( buffer.size() >1 )
    {
        if (!buffer.startsWith("curtain"))
        {
            buffer.clear();
            break;
        }

        udpcurtain_answer* ans = ( udpcurtain_answer* ) buffer.data();
        m_curtainStateTracker->setCurtain(ans->position);
        m_curtainStateTracker->setCurtainMax(ans->max);
        emit stateChanged(m_curtainStateTracker);
        buffer = buffer.mid ( sizeof(udpcurtain_answer) );
    }
}

void Controller::error_curtain ( QAbstractSocket::SocketError err )
{
    qDebug() << __FUNCTION__ << __LINE__ << err;
}

void Controller::disconnected_curtain()
{
    m_tryReconnectTimer_curtain.start();
}

void Controller::connected_curtain()
{
    if ( m_udpSocket_curtain->isOpen() ) {
        m_tryReconnectTimer_curtain.stop();
        setCurtain(255);
    }
}

void Controller::setCurtain(unsigned int position)
{
    //if (position>m_curtain_max) return;
    char a = (unsigned char) position;
    m_udpSocket_curtain->write( &a, 1 );
    m_curtainStateTracker->setCurtain(position);
    emit stateChanged(m_curtainStateTracker);
}

unsigned int Controller::getCurtain()
{
    return m_curtainStateTracker->curtain();
}

void Controller::connectTo ( QHostAddress host, int udpLed, int udpCurtain )
{
    if (udpLed!=0) {
        m_udpport_lights = udpLed;
        m_host = host;
        if ( m_udpSocket_lights ) delete m_udpSocket_lights;
        m_udpSocket_lights = new QUdpSocket ( this );
        connect ( m_udpSocket_lights,SIGNAL ( connected() ),SLOT ( connected_lights() ) );
        connect ( m_udpSocket_lights,SIGNAL ( disconnected() ),SLOT ( disconnected_lights() ) );
        connect ( m_udpSocket_lights,SIGNAL ( readyRead() ),SLOT ( readyRead_lights() ) );
        connect ( m_udpSocket_lights,SIGNAL ( error ( QAbstractSocket::SocketError ) ),SLOT ( error_lights ( QAbstractSocket::SocketError ) ) );
        m_udpSocket_lights->connectToHost ( host, udpLed );
    }
    if (udpCurtain!=0) {
        m_udpport_curtain = udpCurtain;
        if ( m_udpSocket_curtain ) delete m_udpSocket_curtain;
        m_udpSocket_curtain = new QUdpSocket ( this );
        connect ( m_udpSocket_curtain,SIGNAL ( connected() ),SLOT ( connected_curtain() ) );
        connect ( m_udpSocket_curtain,SIGNAL ( disconnected() ),SLOT ( disconnected_curtain() ) );
        connect ( m_udpSocket_curtain,SIGNAL ( readyRead() ),SLOT ( readyRead_curtain() ) );
        connect ( m_udpSocket_curtain,SIGNAL ( error ( QAbstractSocket::SocketError ) ),SLOT ( error_curtain ( QAbstractSocket::SocketError ) ) );
        m_udpSocket_curtain->connectToHost ( host, udpCurtain );
    }
}

void Controller::reconnect_lights()
{
    connectTo ( m_host, m_udpport_lights, 0 );
}

void Controller::readyRead_lights()
{
    if ( ! m_udpSocket_lights->bytesAvailable() ) return;
    QByteArray buffer = m_udpSocket_lights->readAll();
    QSettings settings;
	settings.beginGroup ( QLatin1String("channelnames") );
	while ( buffer.size() >1 )
    {
        if (!buffer.startsWith("stella"))
        {
            buffer.clear();
            break;
        }

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
                emit stateChanged(cv);
                ChannelNameStateTracker* cn = new ChannelNameStateTracker();
                m_channelnames.append(cn);
                cn->setChannel(i);
                cn->setValue(name);
                emit stateChanged(cn);
            }
            else if ( m_channelvalues[i]->value() != newvalue )
            {
                m_channelvalues[i]->setValue(newvalue);
                emit stateChanged(m_channelvalues[i]);
            }
        }
        emit dataLoadingComplete();
        buffer = buffer.mid ( ans->channels+7 );
    }
}

void Controller::error_lights ( QAbstractSocket::SocketError err )
{
    qDebug() << __FUNCTION__ << __LINE__ << err;
}

void Controller::disconnected_lights()
{
    m_tryReconnectTimer_lights.start();
    m_sendTimer_lights.stop();
}

void Controller::connected_lights()
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

void Controller::sendTimeout_lights()
{
    QByteArray bytes;
    foreach ( const udpstella_packet d, m_queue_lights )
    {
        bytes.append ( ( const char* ) &d,sizeof ( d ) );
    }
    m_queue_lights.clear();
    m_udpSocket_lights->write( bytes );
}

void Controller::refresh()
{
    connected_lights();
    connected_curtain();
}

QList<AbstractStateTracker*> Controller::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    foreach (ChannelValueStateTracker* p, m_channelvalues) l.append(p);
    foreach (ChannelNameStateTracker* p, m_channelnames) l.append(p);
    l.append(m_curtainStateTracker);
    return l;
}

void Controller::setChannelName ( uint channel, const QString& name )
{
    if ( channel>= ( unsigned int ) m_channelnames.size() ) return;
    m_channelnames[channel]->setValue(name);
    emit stateChanged(m_channelnames[channel]);

    QSettings settings;
    settings.beginGroup ( QLatin1String("channelnames") );
    settings.setValue ( QLatin1String("channel")+QString::number ( channel ), name );
}

unsigned int Controller::getChannel(unsigned int channel) const
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return 300;
    return m_channelvalues.at(channel)->value();
}

void Controller::setChannel ( uint channel, uint value, uint fade )
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    value = qBound ( ( unsigned int ) 0, value, ( unsigned int ) 255 );
    m_channelvalues[channel]->setValue(value);
    emit stateChanged(m_channelvalues[channel]);
    udpstella_packet setch;
    setch.type = fade;
    setch.channel = channel;
    setch.value = value;

    m_queue_lights.insert ( channel, setch );
    m_sendTimer_lights.start();
}

void Controller::inverseChannel(uint channel, uint fade)
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    const unsigned int newvalue = 255 - m_channelvalues[channel]->value();
    setChannel(channel, newvalue, fade);
}

void Controller::setChannelExponential ( uint channel, int multiplikator, uint fade )
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

void Controller::setChannelRelative ( uint channel, int value, uint fade )
{
    if ( channel>= ( unsigned int ) m_channelvalues.size() ) return;
    value += m_channelvalues[channel]->value();
    const unsigned int v = ( unsigned int ) qMin ( 0, value);
    setChannel ( channel, v, fade );
}


int Controller::countChannels()
{
    return m_channelvalues.size();
}

QString Controller::getChannelName(uint channel) {
    if (channel>=(uint)m_channelnames.size()) return QString();
    return m_channelnames[channel]->value();
}
