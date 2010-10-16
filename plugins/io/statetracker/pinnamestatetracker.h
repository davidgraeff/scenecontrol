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

#ifndef PINNAMESTATETRACKER_H
#define PINNAMESTATETRACKER_H
#include "abstractstatetracker.h"

class PinNameStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pin READ pin WRITE setPin);
    Q_PROPERTY(QString value READ value WRITE setValue);
public:
    PinNameStateTracker(QObject* parent = 0);
    unsigned int pin() const;
    void setPin(unsigned int value);
    QString value() const;
    void setValue(const QString& value);
private:
    unsigned int m_pin;
    QString m_value;
};

#endif // PINNAMESTATETRACKER_H
