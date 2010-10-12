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

#ifndef ActorMute_h
#define ActorMute_h

#include "abstractactor.h"


class ActorMute : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue)
	Q_PROPERTY(int mute READ mute WRITE setMute)
	Q_PROPERTY(double volume READ volume WRITE setVolume)
	Q_PROPERTY(bool relative READ relative WRITE setRelative)
public:
    ActorMute(QObject* parent = 0);
    virtual void execute();
    
    const QString& value() const {
        return m_value;
    }
    void setValue( const QString& v ) {
        m_value = v;
    }
	int mute() const {
	    return m_mute;
	}
	
	void setMute( int m ) {
	    m_mute = m;
	}
	
	qreal volume() const {
	    return m_volume;
	}
	
	void setVolume( qreal v ) {
	    m_volume = v;
	}
	
	bool relative() const {
	    return m_relative;
	}
	
	void setRelative( bool r ) {
	    m_relative = r;
	}
private:
    QString m_value;
    int m_mute;
	qreal m_volume;
	bool m_relative;
};

#endif // ActorMute_h
