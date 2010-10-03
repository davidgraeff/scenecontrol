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

#ifndef ActorCinemaPosition_h
#define ActorCinemaPosition_h

#include "abstractactor.h"


class ActorCinemaPosition : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue);
    
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorCinemaPosition(QObject* parent = 0);
    virtual void execute();
    qreal value() const ;
    void setValue(qreal value) ;
    
    
    bool relative() const { return m_relative; }
    void setRelative(bool value) { m_relative = value; }
private:
    qreal m_volume;
    
    bool m_relative;
};

#endif // ActorCinemaPosition_h
