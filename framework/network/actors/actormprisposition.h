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

#ifndef ActorMprisPosition_h
#define ActorMprisPosition_h

#include "abstractactor.h"

class ActorMprisPosition : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue);
    Q_PROPERTY(QString mprisid READ mprisid WRITE setMprisid);
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorMprisPosition(QObject* parent = 0);
    qreal value() const ;
    void setValue(qreal value) ;
    QString mprisid() const ;
    void setMprisid(const QString& value) ;
    bool relative() const { return m_relative; }
    void setRelative(bool value) { m_relative = value; }
    virtual void changed() ;
private:
    qreal m_volume;
    QString m_mprisid;
    bool m_relative;
};

#endif // ActorMprisPosition_h
