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

#ifndef ACTORLEDNAMESERVICEPROVIDER_H
#define ACTORLEDNAMESERVICEPROVIDER_H

#include "shared/abstractserviceprovider.h"


class ActorLedName : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString ledname READ ledname WRITE setLedname);
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);
  public:
    ActorLedName(QObject* parent = 0);
    virtual void execute();
    QString ledname() const;
    void setLedname(const QString& ledname);
    unsigned int channel() const;
    void setChannel(unsigned int channel);
  private:
    QString m_ledname;
    unsigned int m_channel;
};

#endif // ACTORLEDNAMESERVICEPROVIDER_H
