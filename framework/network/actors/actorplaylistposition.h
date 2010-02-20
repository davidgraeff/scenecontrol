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

#ifndef ActorPlaylistPosition_h
#define ActorPlaylistPosition_h

#include "abstractactor.h"

class ActorPlaylistPosition : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue);
    Q_PROPERTY(bool relative READ relative WRITE setRelative);
public:
    ActorPlaylistPosition(QObject* parent = 0);
    qreal value() const { return m_position; }
    void setValue(qreal value) { m_position = value; }
    bool relative() const { return m_relative; }
    void setRelative(bool value) { m_relative = value; }
    virtual void changed() ;
private:
    qreal m_position;
    bool m_relative;
};

#endif // ActorPlaylistPosition_h
