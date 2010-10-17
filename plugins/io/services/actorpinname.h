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

#ifndef ACTORPINNAMESERVICEPROVIDER_H
#define ACTORPINNAMESERVICEPROVIDER_H

#include "shared/abstractserviceprovider.h"


class ActorPinName : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString pinname READ pinname WRITE setPinname);
    Q_PROPERTY(unsigned int pin READ pin WRITE setPin);
public:
    ActorPinName(QObject* parent = 0);
    virtual ProvidedTypes providedtypes(){return ActionType;}
    QString pinname() const;
    void setPinname(const QString& value);
    unsigned int pin() const;
    void setPin(unsigned int value);
private:
    QString m_pinname;
    unsigned int m_pin;
};

#endif // ACTORPINNAMESERVICEPROVIDER_H
