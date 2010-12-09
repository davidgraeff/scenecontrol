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
    virtual QString service_name() {
        return tr("Setzt LED");
    }
    virtual QString service_desc() {
        return tr("Setzt eine LED auf den angegebene Wert auf die angegebene Weise");
    }
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Kanal");
        case 1:
            switch (enumindex) {
            case 0:
                return tr("Sofort");
            case 1:
                return tr("Faden");
            case 2:
                return tr("Flashy Faden");
            default:
                return tr("Farbübernahme");
            }
        case 2:
            switch (enumindex) {
            case 0:
                return tr("Absolut");
            case 1:
                return tr("Relativ");
            case 2:
                return tr("Invers (ignoriert Wert)");
            case 3:
                return tr("Multiplikator");
            default:
                return tr("Art");
            }
        case 3:
            return tr("Wert");
        default:
            return QString();
        }
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
    ActorLed::fadetypeEnum m_fadetype;
    ActorLed::assignmentEnum m_assignment;
	int m_value;
};

#endif // ACTORSERVICEPROVIDER_H
