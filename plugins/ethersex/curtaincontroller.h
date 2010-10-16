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

#ifndef CURTAINCONTROLLER_H
#define CURTAINCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>

/**
Daten von ethersex abfragen und cachen
*/

class AbstractStateTracker;
class CurtainStateTracker;

struct udpcurtain_answer {
    char id[7];
    uint8_t position;
    uint8_t max;
};

class CurtainController : public QObject
{
    Q_OBJECT
public:
    /**
    Daten von ethersex abrufen
    */
    CurtainController();
    ~CurtainController();
    void connectTo(QHostAddress host, int udpport);
    void refresh();
    QList<AbstractStateTracker*> getStateTracker();
    void setCurtain(unsigned int position);
    unsigned int getCurtain();
    
private:
    // CURTAIN //
    int m_udpport_curtain;
    QHostAddress m_host_curtain;
    QUdpSocket* m_udpSocket_curtain;
    QTimer m_tryReconnectTimer_curtain;
    CurtainStateTracker* m_curtainStateTracker;

private Q_SLOTS:
    // CURTAIN //
    void readyRead_curtain();
    void disconnected_curtain();
    void connected_curtain();
    void error_curtain(QAbstractSocket::SocketError);
    void reconnect_curtain();

Q_SIGNALS:
    void dataAvailable();
};

#endif // CURTAINCONTROLLER_H
