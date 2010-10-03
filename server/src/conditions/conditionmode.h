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

#ifndef ConditionMode_h
#define ConditionMode_h
#include "abstractcondition.h"

class ConditionMode : public AbstractCondition
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue);
public:
    ConditionMode(QObject* parent = 0);
    virtual bool ok();
    QString value() const;
    void setValue(QString value);
private:
    QString m_value;
};

#endif // ConditionMode_h
