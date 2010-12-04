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

#ifndef ACTORPLAYLISTVOLUMESERVICEPROVIDER_H
#define ACTORPLAYLISTVOLUMESERVICEPROVIDER_H
#include <shared/abstractserviceprovider.h>

class ActorPlaylistVolume : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume);
	Q_CLASSINFO("volume_doublemin", "-1.0");
	Q_CLASSINFO("volume_doublemax", "1.0");
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorPlaylistVolume(QObject* parent = 0);
	virtual QString service_name(){return tr("Abspiellistenlautstärke");}
	virtual QString service_desc(){return tr("Regelt die Lautstärke einer Abspielliste");}
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
    qreal volume() const ;
    void setVolume(qreal value) ;
    bool relative() const ;
    void setRelative(bool value) ;
private:
    qreal m_volume;
    bool m_relative;
};

#endif // ACTORPLAYLISTVOLUMESERVICEPROVIDER_H
