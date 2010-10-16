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

#ifndef CurtainStateTracker_h
#define CurtainStateTracker_h
#include "abstractstatetracker.h"

class CurtainStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(unsigned int curtain READ curtain WRITE setCurtain);
    Q_PROPERTY(unsigned int curtainMax READ curtainMax WRITE setCurtainMax);
public:
    CurtainStateTracker(QObject* parent = 0);
    unsigned int curtain() const;
    unsigned int curtainMax() const;
    void setCurtain(unsigned int v);
    void setCurtainMax(unsigned int v);
private:
    unsigned int m_curtain;
    unsigned int m_curtainmax;
};

#endif // CurtainStateTracker_h
