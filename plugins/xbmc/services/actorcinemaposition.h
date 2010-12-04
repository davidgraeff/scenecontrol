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

#include "shared/abstractserviceprovider.h"


class ActorCinemaPosition : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue);
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorCinemaPosition(QObject* parent = 0);
	virtual QString service_name(){return tr("Abspielposition Cinema setzen");}
	virtual QString service_desc(){return tr("Setzt die aktuelle Abspielposition der Vorführung");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Position");
        case 1:
            return tr("Relativ");
        default:
            return QString();
        }
    }
    qreal value() const ;
    void setValue(qreal value) ;
    bool relative() const { return m_relative; }
    void setRelative(bool value) { m_relative = value; }
private:
    qreal m_volume;
    bool m_relative;
};

#endif // ActorCinemaPosition_h
