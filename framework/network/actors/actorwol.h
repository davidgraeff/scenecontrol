/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "abstractactor.h"

class ActorWOL : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(QString mac READ mac WRITE setMac)
public:
    ActorWOL(QObject* parent = 0);
    virtual void changed() ;
    
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
