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

class ActorLed : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);
    Q_CLASSINFO("channel_model", "ChannelsModel")
    Q_CLASSINFO("channel_model_displaytype", "0");
    Q_CLASSINFO("channel_model_savetype", "32");
    Q_PROPERTY(ActorLed::fadetypeEnum fadetype READ fadetype WRITE setFadetype);
    Q_PROPERTY(ActorLed::assignmentEnum assignment READ assignment WRITE setAssignment);
    Q_PROPERTY(int value READ value WRITE setValue);
    Q_CLASSINFO("value_max", "255");
    Q_CLASSINFO("value_min", "0");
public:
    enum assignmentEnum
    {
        ValueAbsolute,
        ValueRelative,
        ValueInverse,
        ValueMultiplikator
    };
    Q_ENUMS(assignmentEnum);
    enum fadetypeEnum
    {
        FadeTypeImmediately,
        FadeTypeFade,
	FadeTypeFlashyFade
    };
    Q_ENUMS(fadetypeEnum);
    ActorLed(QObject* parent = 0);
    virtual ProvidedTypes providedtypes() {
        return ActionType;
    }
    void setChannel(unsigned int channel);
    unsigned int channel() const;
    void setFadetype(ActorLed::fadetypeEnum fadetype);
    ActorLed::fadetypeEnum fadetype() const;
    void setValue(int value);
    int value() const;
    void setAssignment(ActorLed::assignmentEnum value);
    ActorLed::assignmentEnum assignment() const;
private:
    unsigned int m_channel;
    int m_value;
    ActorLed::fadetypeEnum m_fadetype;
    ActorLed::assignmentEnum m_assignment;
};

#endif // ACTORSERVICEPROVIDER_H
