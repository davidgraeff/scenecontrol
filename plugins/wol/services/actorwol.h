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

#ifndef ACTORWOL_H
#define ACTORWOL_H

#include "shared/abstractserviceprovider.h"

class ActorWOL : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString mac READ mac WRITE setMac)
public:
    ActorWOL(QObject* parent = 0);
	virtual QString service_name(){return tr("WakeUpOnLan");}
	virtual QString service_desc(){return tr("Schickt ein WOL Paket an einen besteimmten PC");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("MAC");
        default:
            return QString();
        }
    }
    const QString& mac() const {
        return m_mac;
    }
    
    void setMac( const QString& m ) {
        m_mac = m;
    }
private:
    QString m_mac;
};
#endif // ACTORWOL_H
