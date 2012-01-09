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

#pragma once
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>

class AbstractPlugin;

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
    Controller(AbstractPlugin* plugin);
    ~Controller();
    void connectToLeds(const QString& host, int port);

    int countChannels();
    QString getChannelName ( const QString& channel );
    void setChannel ( const QString& channel, int value, uint fade );
    void setChannelName ( const QString& channel, const QString& name );
    void inverseChannel(const QString& channel, uint fade);
    void setChannelExponential ( const QString& channel, int multiplikator, uint fade );
    void setChannelRelative ( const QString& channel, int value, uint fade );
    unsigned int getChannel(const QString& channel) const;
    void moodlight(const QString& channel, bool moodlight);

    struct ledchannel {
        int value;
        QString name;
        bool moodlight;
        uint8_t channel;
        ledchannel(uint8_t channel, int value, const QString& name) {
            this->channel = channel;
            this->value = value;
            moodlight = false;
            this->name = name;
        }
        ledchannel() {
            value = -1;
            moodlight = false;
        }
    };
    QMap<QString,ledchannel> m_leds;
private:
    AbstractPlugin* m_plugin;

    int m_channels;
    QTimer m_moodlightTimer;
    // udp
    int m_sendPort;
    QUdpSocket *m_socket;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
    void moodlightTimeout();
Q_SIGNALS:
    void ledChanged(const QString& id, const QString& name, int value);
    void dataLoadingComplete();
    void ledsCleared();
};
