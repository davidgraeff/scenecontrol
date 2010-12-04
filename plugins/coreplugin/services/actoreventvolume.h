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
#include <shared/abstractserviceprovider.h>

class ActorEventVolume : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume);
	Q_CLASSINFO("volume_doublemin", "-1.0");
	Q_CLASSINFO("volume_doublemax", "1.0");
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorEventVolume(QObject* parent = 0);
	virtual QString service_name(){return tr("Ereignislautstärke");}
	virtual QString service_desc(){return tr("Setzt die Laustärke für Ereignisse");}

    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Lautstärke");
        case 1:
            return tr("Relativ");
        default:
            return QString();
        }
    }
    void setVolume(qreal volume);
    qreal volume();
    bool relative() const ;
    void setRelative(bool value) ;
private:
    qreal m_volume;
    bool m_relative;
};
