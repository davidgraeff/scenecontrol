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

#ifndef ConditionLed_h
#define ConditionLed_h
#include "abstractcondition.h"

class ConditionLed : public AbstractCondition
{
    Q_OBJECT
    Q_PROPERTY(unsigned int min READ max WRITE setMin);
    Q_PROPERTY(unsigned int max READ max WRITE setMax);
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);

public:
    ConditionLed(QObject* parent = 0);
    virtual bool ok();
    unsigned int channel() const;
    void setChannel(unsigned int value);
    unsigned int min() const;
    void setMin(unsigned int value);
    unsigned int max() const;
    void setMax(unsigned int value);
  private:
    unsigned int m_channel;
    unsigned int m_min;
    unsigned int m_max;
};

#endif // ConditionLed_h
