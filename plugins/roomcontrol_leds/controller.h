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
class QextSerialPort;
class myPluginExecute;
class CurtainStateTracker;
class ChannelNameStateTracker;
class AbstractStateTracker;
class ChannelValueStateTracker;

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
    void connectToLeds(const QString& url);
    void setCurtain(unsigned int position);
    int getCurtain();

    int countChannels();
    QString getChannelName ( uint channel );
    void setChannel ( uint channel, uint value, uint fade );
    void setChannelName ( uint channel, const QString& name );
    void inverseChannel(uint channel, uint fade);
    void setChannelExponential ( uint channel, int multiplikator, uint fade );
    void setChannelRelative ( uint channel, int value, uint fade );
    unsigned int getChannel(unsigned int channel) const;
private:
    AbstractPlugin* m_plugin;
	struct ledchannel {
		int value;
		QString name;
		ledchannel() { value = 300; }
	};
    QMap<int,ledchannel> m_leds;
	int m_curtain_max;
	int m_curtain_value;
    
    int m_channels;
    QByteArray m_buffer;
    void parseLeds(const QByteArray& data);
    void parseCurtain(unsigned char current, unsigned char max);
    void parseInit(unsigned char protocolversion);
    void parseSensors(unsigned char s1, unsigned char s2);
    QTimer m_panicTimer;
    int m_bufferpos;
    enum readStateEnum {
        ReadOK,
        ReadEnd
    };
    readStateEnum m_readState;
    // rs232 special
    QextSerialPort* m_serial;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
    void panicTimeout();
Q_SIGNALS:
    void curtainChanged(int current, int max);
	void ledvalueChanged(int channel, int value);
	void lednameChanged(int channel, const QString& name);
    void dataLoadingComplete();
};
