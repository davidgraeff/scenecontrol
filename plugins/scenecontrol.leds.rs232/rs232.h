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
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>
#include <shared/serialdevice/qxtserialdevice.h>

enum stella_fade_function
{
    STELLA_SET_IMMEDIATELY,
    STELLA_SET_FADE,
    STELLA_SET_FLASHY,
    STELLA_SET_IMMEDIATELY_RELATIVE,
    STELLA_SET_MOODLIGHTED, // only relevant for udp stella protocoll
    STELLA_GETALL = 255
};

class rs232leds : public QObject
{
    Q_OBJECT
public:
	rs232leds();
	~rs232leds();
	void disconnectLeds();
    void connectToLeds(const QString& url);

    struct ledchannel {
        int value;
        ledchannel() {
            value = 300;
        }
    };
    QMap<QString,ledchannel> m_leds;
    int m_curtain_max;
    int m_curtain_value;
	inline bool channelExist(const QString& channel) {return m_leds.contains(channel);}
public Q_SLOTS:
    void setCurtain(unsigned int position);
    int getCurtain();

    void setLed ( const QString& channel, int value, int fade );
    int getLed( const QString& channel ) const;
    int countLeds();
private Q_SLOTS:
    void readyRead();
    void panicTimeout();
Q_SIGNALS:
	void ledChanged ( QString channel, int value);
	void curtainChanged ( int current, int max );
	void connectedToLeds(unsigned char protocolversion);
private:
	bool m_connected;
	QByteArray m_buffer;
	void parseLeds(const QByteArray& data, int channels);
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
};
