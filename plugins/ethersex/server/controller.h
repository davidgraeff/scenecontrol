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

#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

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

class CurtainStateTracker;
class ChannelNameStateTracker;
class AbstractStateTracker;
class ChannelValueStateTracker;
struct udpstella_packet
{
    uint8_t type;    // see above
    uint8_t channel; // if port: pin
    uint8_t value;
};

struct udpcurtain_answer {
    char id[7];
    uint8_t position;
    uint8_t max;
};

struct udpstella_answer {
    char id[6];
    uint8_t channels;
    uint8_t channel[16];
};

enum stella_fade_function
{
  STELLA_SET_IMMEDIATELY,
  STELLA_SET_FADE,
  STELLA_SET_FLASHY,
  STELLA_SET_IMMEDIATELY_RELATIVE,
  STELLA_SET_MOODLIGHTED, // only relevant for udp stella protocoll
  STELLA_GETALL = 255
};

class Controller : public QObject
{
    Q_OBJECT
public:
    /**
    Daten von ethersex abrufen
    */
    Controller();
    ~Controller();
    void connectTo(QHostAddress host, int udpLed, int udpCurtain);
    void refresh();
    QList<AbstractStateTracker*> getStateTracker();
    void setCurtain(unsigned int position);
    unsigned int getCurtain();

    int countChannels();
    QString getChannelName ( uint channel );
    void setChannel ( uint channel, uint value, uint fade );
    void setChannelName ( uint channel, const QString& name );
    void inverseChannel(uint channel, uint fade);
    void setChannelExponential ( uint channel, int multiplikator, uint fade );
    void setChannelRelative ( uint channel, int value, uint fade );
    unsigned int getChannel(unsigned int channel) const;
private:
    // LIGHTS //
    int m_udpport_lights;
    QHostAddress m_host;
    QUdpSocket* m_udpSocket_lights;
    QTimer m_sendTimer_lights;
    QTimer m_tryReconnectTimer_lights;
    QMap<uint, udpstella_packet > m_queue_lights;
    QList<ChannelValueStateTracker*> m_channelvalues;
    QList<ChannelNameStateTracker*> m_channelnames;
    // CURTAIN //
    int m_udpport_curtain;
    QUdpSocket* m_udpSocket_curtain;
    QTimer m_tryReconnectTimer_curtain;
    CurtainStateTracker* m_curtainStateTracker;
private Q_SLOTS:
    // LIGHTS //
    void sendTimeout_lights();
    void readyRead_lights();
    void disconnected_lights();
    void connected_lights();
    void error_lights(QAbstractSocket::SocketError);
    void reconnect_lights();
    // CURTAIN //
    void readyRead_curtain();
    void disconnected_curtain();
    void connected_curtain();
    void error_curtain(QAbstractSocket::SocketError);
    void reconnect_curtain();
Q_SIGNALS:
    void stateChanged(AbstractStateTracker*);
};

#endif // LEDCONTROLLER_H
