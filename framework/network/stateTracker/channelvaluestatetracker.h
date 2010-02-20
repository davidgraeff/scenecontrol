/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef CHANNELVALUESERVICEPROVIDER_H
#define CHANNELVALUESERVICEPROVIDER_H
#include "abstractstatetracker.h"

class ChannelValueStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);
    Q_PROPERTY(unsigned int value READ value WRITE setValue);
public:
    ChannelValueStateTracker(QObject* parent = 0);
    unsigned int channel() const;
    void setChannel(unsigned int value);
    unsigned int value() const;
    void setValue(unsigned int value);
private:
    unsigned int m_channel;
    unsigned int m_value;
};

#endif // CHANNELVALUESERVICEPROVIDER_H
