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

#ifndef ActorMute_h
#define ActorMute_h

#include "abstractactor.h"


class ActorMute : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue);
    Q_PROPERTY(bool mute READ mute WRITE setMute);
public:
    ActorMute(QObject* parent = 0);
    qreal value() const ;
    void setValue(qreal value) ;
    bool mute() const { return m_mute; }
    void setMute(bool value) { m_mute = value; }
    virtual void changed() ;
private:
    qreal m_value;
    bool m_mute;
};

#endif // ActorMute_h
