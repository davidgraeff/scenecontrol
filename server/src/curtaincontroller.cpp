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

#include "curtaincontroller.h"
#include <bitset>
#include "RoomControlServer.h"

#define _BV(bit) (1<<(bit))
#include <QSettings>
#include "stateTracker/curtainstatetracker.h"

CurtainController::CurtainController()
{
    m_curtainStateTracker = new CurtainStateTracker();

    m_udpSocket_curtain = 0;
    m_tryReconnectTimer_curtain.setInterval ( 10000 );
    connect ( &m_tryReconnectTimer_curtain,SIGNAL ( timeout() ),SLOT ( reconnect_curtain() ) );
}

CurtainController::~CurtainController()
{
    delete m_curtainStateTracker;
    m_udpSocket_curtain->disconnectFromHost();
}

void CurtainController::connectTo(QHostAddress host, int udpport)
{
    m_udpport_curtain = udpport;
    m_host_curtain = host;
    if ( m_udpSocket_curtain ) delete m_udpSocket_curtain;
    m_udpSocket_curtain = new QUdpSocket ( this );
    connect ( m_udpSocket_curtain,SIGNAL ( connected() ),SLOT ( connected_curtain() ) );
    connect ( m_udpSocket_curtain,SIGNAL ( disconnected() ),SLOT ( disconnected_curtain() ) );
    connect ( m_udpSocket_curtain,SIGNAL ( readyRead() ),SLOT ( readyRead_curtain() ) );
    connect ( m_udpSocket_curtain,SIGNAL ( error ( QAbstractSocket::SocketError ) ),SLOT ( error_curtain ( QAbstractSocket::SocketError ) ) );
    m_udpSocket_curtain->connectToHost ( host, udpport );
}


void CurtainController::reconnect_curtain()
{
    connectTo ( m_host_curtain, m_udpport_curtain );
}

void CurtainController::readyRead_curtain()
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
        m_curtainStateTracker->sync();
        buffer = buffer.mid ( sizeof(udpcurtain_answer) );
    }
}

void CurtainController::error_curtain ( QAbstractSocket::SocketError err )
{
    qDebug() << __FUNCTION__ << __LINE__ << err;
}

void CurtainController::disconnected_curtain()
{
    m_tryReconnectTimer_curtain.start();
}

void CurtainController::connected_curtain()
{
    if ( m_udpSocket_curtain->isOpen() ) {
        m_tryReconnectTimer_curtain.stop();
        setCurtain(255);
    }
}

void CurtainController::refresh()
{
    connected_curtain();
}

QList<AbstractStateTracker*> CurtainController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    l.append(m_curtainStateTracker);
    return l;
}

void CurtainController::setCurtain(unsigned int position)
{
    //if (position>m_curtain_max) return;
    char a = (unsigned char) position;
    m_udpSocket_curtain->write( &a, 1 );
    m_curtainStateTracker->setCurtain(position);
    m_curtainStateTracker->sync();
}

unsigned int CurtainController::getCurtain()
{
    return m_curtainStateTracker->curtain();
}
