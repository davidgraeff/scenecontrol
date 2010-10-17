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

#ifndef ACTORLEDSERVICEPROVIDER_H
#define ACTORLEDSERVICEPROVIDER_H
#include "shared/abstractserviceprovider.h"

enum ActorLedEnum
{
    ValueAbsolute,
    ValueRelative,
    ValueInverse,
    ValueMultiplikator
};
Q_ENUMS(ActorLedEnum);

class ActorLed : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);
    Q_PROPERTY(unsigned int fadetype READ fadetype WRITE setFadetype);
    Q_PROPERTY(int assignment READ assignment WRITE setAssignment);
    Q_PROPERTY(int value READ value WRITE setValue);
public:
    ActorLed(QObject* parent = 0);
    virtual void execute();
    void setChannel(unsigned int channel);
    unsigned int channel() const;
    void setFadetype(unsigned int fadetype);
    unsigned int fadetype() const;
    void setValue(int value);
    int value() const;
    void setAssignment(int value);
    int assignment() const;
private:
    unsigned int m_channel;
    int m_value;
    unsigned int m_fadetype;
    int m_assignment;
};

#endif // ACTORSERVICEPROVIDER_H
