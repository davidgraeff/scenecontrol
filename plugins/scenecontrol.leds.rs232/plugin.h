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
#include "shared/plugins/abstractplugin.h"
#include "rs232.h"
#include <QStringList>
#include <QObject>
#include <QMap>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin( const QString& pluginid, const QString& instanceid );
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
private:
	rs232leds m_leds;
    void connectToLeds(const QString& host, int port);
private Q_SLOTS:
	void ledChanged ( QString channel, int value);
	void curtainChanged ( int current, int max );
	void connectedToLeds(unsigned char protocolversion);
public Q_SLOTS:
	inline int countChannels() {return m_leds.countLeds();}
    void setLed ( const QString& channel, int value, int fade );
    void toggleLed(const QString& channel, int fade);
    void setLedExponential ( const QString& channel, int multiplikator, int fade );
    void setLedRelative ( const QString& channel, int value, int fade );
    int getLed(const QString& channel) const;
    bool isLedValue( const QString& channel, int lower, int upper );
};
