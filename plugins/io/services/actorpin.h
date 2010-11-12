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

#ifndef ACTORPINSERVICEPROVIDER_H
#define ACTORPINSERVICEPROVIDER_H

#include "shared/abstractserviceprovider.h"

class ActorPin : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pin READ pin WRITE setPin);
    Q_CLASSINFO("pin_model", "PinsModel")
    Q_CLASSINFO("pin_model_displaytype", "0");
    Q_CLASSINFO("pin_model_savetype", "32");
    Q_PROPERTY(ActorPin::ActorPinEnum value READ value WRITE setValue);

public:
    enum ActorPinEnum
    {
        PinOff,
        PinOn,
        PinToggle
    };
    Q_ENUMS(ActorPinEnum);
    ActorPin(QObject* parent = 0);
    virtual ProvidedTypes providedtypes() {
        return ActionType;
    }
    unsigned int pin() const;
    void setPin(unsigned int value);
    ActorPin::ActorPinEnum value() const;
    void setValue(ActorPin::ActorPinEnum value);
private:
    unsigned int m_pin;
    ActorPin::ActorPinEnum m_value;
};

#endif // ACTORPINSERVICEPROVIDER_H
