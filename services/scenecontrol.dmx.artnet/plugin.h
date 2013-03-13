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

#pragma once
#include "abstractplugin.h"
#include <QStringList>
#include <QObject>
#include <QMap>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>

enum stella_fade_function
{
    STELLA_SET_IMMEDIATELY,
    STELLA_SET_FADE,
    STELLA_SET_FLASHY,
    STELLA_SET_IMMEDIATELY_RELATIVE,
    STELLA_SET_MOODLIGHTED, // only relevant for udp stella protocoll
    STELLA_GETALL = 255
};

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin( const QString& pluginid, const QString& instanceid );
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties();
    virtual void instanceConfiguration(const QVariantMap& data);
private:
    void connectToLeds(const QString& host, int port);
    void ledChanged(QString channel, int value);

    struct ledchannel {
        int value;
        QString name;
        uint8_t channel;
        ledchannel(uint8_t channel, int value) {
            this->channel = channel;
            this->value = value;
        }
        ledchannel() {
            value = -1;
        }
    };
    QMap<QString,ledchannel> m_leds;

    int m_channels;
    // udp
    int m_sendPort;
    QUdpSocket *m_socket;
    int m_connectTime;
    QTimer m_connectTimer;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
    void resendConnectSequence();
public Q_SLOTS:
    int countChannels();
    void setLed ( const QString& channel, int value, int fade );
    void toggleLed(const QString& channel, int fade);
    void setLedExponential ( const QString& channel, int multiplikator, int fade );
    void setLedRelative ( const QString& channel, int value, int fade );
    int getLed(const QString& channel) const;
    bool isLedValue( const QString& channel, int lower, int upper );
};
