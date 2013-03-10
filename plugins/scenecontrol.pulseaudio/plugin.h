/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010-2012  David Gräff
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
#include <QStringList>
#include "shared/plugins/abstractplugin.h"
#include "pulseController.h"

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:

    virtual ~plugin();
private:
	virtual void initialize();
    virtual void requestProperties ( int sessionid );
	
	void pulseVersion ( int protocol, int server );
	void pulseSinkChanged ( const PulseChannel& channel );
public Q_SLOTS:
	void pulsechannelmute( const QByteArray& sinkid, bool mute );
	void pulsechannelmutetoggle( const QByteArray& sinkid );
	void pulsechannelvolume( const QByteArray& sinkid, double volume, bool relative );
};
