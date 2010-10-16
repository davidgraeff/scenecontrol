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

#ifndef PASTATETRACKER_H
#define PASTATETRACKER_H
#include "abstractstatetracker.h"

class PAStateTracker : public AbstractStateTracker
{
    Q_OBJECT
	Q_PROPERTY(QString sinkname READ sinkname WRITE setSinkname)
	Q_PROPERTY(double volume READ volume WRITE setVolume)
	Q_PROPERTY(bool mute READ mute WRITE setMute)
public:
    PAStateTracker(const QString& sinkname, QObject* parent = 0);
	
	const QString& sinkname() const {
	    return m_sinkname;
	}
	
	void setSinkname( const QString& s ) {
	    m_sinkname = s;
	}
	
	double volume() const {
	    return m_volume;
	}
	
	void setVolume( double v ) {
	    m_volume = v;
	}
	
	bool mute() const {
	    return m_mute;
	}
	
	void setMute( bool m ) {
	    m_mute = m;
	}
private:
	QString m_sinkname;
	double m_volume;
	bool m_mute;
};

#endif // PASTATETRACKER_H
