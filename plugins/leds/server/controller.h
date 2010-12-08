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
	Controller(myPluginExecute* plugin);
    ~Controller();
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
	QString m_pluginname;
    QList<ChannelValueStateTracker*> m_values;
    QList<ChannelNameStateTracker*> m_names;
    CurtainStateTracker* m_curtainStateTracker;
	int m_channels;
	QByteArray m_buffer;
	void determineChannels(const QByteArray& data);
	// rs232 special
	QextSerialPort* m_serial;
	QTimer m_panicTimer;
	int m_bufferpos;
	enum readStateEnum {
		ReadOK,
		ReadEnd
	};
	readStateEnum m_readState;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
	void panicTimeout();
Q_SIGNALS:
    void stateChanged(AbstractStateTracker*);
	void dataLoadingComplete();
};
