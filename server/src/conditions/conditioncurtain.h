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

#ifndef ConditionCurtain_h
#define ConditionCurtain_h
#include "abstractcondition.h"

class ConditionCurtain : public AbstractCondition
{
    Q_OBJECT
    Q_PROPERTY ( unsigned int value READ value WRITE setValue );
public:
    ConditionCurtain(QObject* parent = 0);
    virtual bool ok();
    unsigned int value() const;
    void setValue ( unsigned int v );
private:
    unsigned int m_value;
};

#endif // ConditionCurtain_h
