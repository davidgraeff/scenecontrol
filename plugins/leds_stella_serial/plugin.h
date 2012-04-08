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
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>
#include "_sharedsrc/abstractplugin.h"
#include <shared/qxtserialdevice/qxtserialdevice.h>


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
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
private:
    void connectToLeds(const QString& url);
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
    void ledChanged ( QString channel, int value);
    void curtainChanged ( int current, int max );

    struct ledchannel {
        int value;
        ledchannel() {
            value = 300;
        }
    };
    QMap<QString,ledchannel> m_leds;
    int m_curtain_max;
    int m_curtain_value;

    bool m_connected;
    int m_channels;
    QByteArray m_buffer;
    void parseLeds(const QByteArray& data);
    void parseCurtain(unsigned char current, unsigned char max);
    void parseInit(unsigned char protocolversion);
    void parseSensors(unsigned char s1);
    QTimer m_panicTimer;
    int m_bufferpos;
    enum readStateEnum {
        ReadOK,
        ReadEnd
    };
    readStateEnum m_readState;
    // rs232 special
    QxtSerialDevice* m_serial;
    bool m_panicTimeoutAck;

public Q_SLOTS:
    void setCurtain(unsigned int position);
    int getCurtain();
    bool isCurtainInPosition( int lower, int upper );

    void setLed ( const QString& channel, int value, int fade );
    void setLedExponential ( const QString& channel, int multiplikator, int fade );
    void setLedRelative ( const QString& channel, int value, int fade );
    void toggleLed ( const QString& channel, int fade );
    int getLed( const QString& channel ) const;
    bool isLedValue( const QString& channel, int lower, int upper );
    int countLeds();
private Q_SLOTS:
    void readyRead();
    void panicTimeout();
};
