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
#include <shared/abstractserviceprovider.h>

class ConditionLed : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);
    Q_CLASSINFO("channel_model", "ChannelsModel")
    Q_CLASSINFO("channel_model_displaytype", "0");
    Q_CLASSINFO("channel_model_savetype", "32");
    Q_PROPERTY(unsigned int min READ max WRITE setMin);
    Q_CLASSINFO("min_max", "255");
    Q_CLASSINFO("min_min", "0");
    Q_PROPERTY(unsigned int max READ max WRITE setMax);
    Q_CLASSINFO("max_max", "255");
    Q_CLASSINFO("max_min", "0");
public:
    ConditionLed(QObject* parent = 0);
	virtual QString service_name(){return tr("Ledbedingung");}
	virtual QString service_desc(){return tr("Bedingung ob eine LED einen gewissen Wert hat");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Kanal");
        case 1:
            return tr("Minimum");
        case 2:
            return tr("Maximum");
        default:
            return QString();
        }
    }
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
