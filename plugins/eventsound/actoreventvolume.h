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

#ifndef ACTOREVENTVOLUMESERVICEPROVIDER_H
#define ACTOREVENTVOLUMESERVICEPROVIDER_H

#include "abstractactor.h"

class ActorEventVolume : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume);
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorEventVolume(QObject* parent = 0);
    void setVolume(qreal volume);
    qreal volume();
    bool relative() const ;
    void setRelative(bool value) ;

    virtual void execute();
private:
    qreal m_volume;
    bool m_relative;
};

#endif // ACTOREVENTVOLUMESERVICEPROVIDER_H
