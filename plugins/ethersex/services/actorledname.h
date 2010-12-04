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
    Q_PROPERTY(unsigned int channel READ channel WRITE setChannel);
    Q_CLASSINFO("channel_model", "ChannelsModel")
    Q_CLASSINFO("channel_model_displaytype", "0");
    Q_CLASSINFO("channel_model_savetype", "32");
    Q_PROPERTY(QString ledname READ ledname WRITE setLedname);
  public:
    ActorLedName(QObject* parent = 0);
	virtual QString service_name(){return tr("Lednamen setzen");}
	virtual QString service_desc(){return tr("Setzt den Namen zu einer LED");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Kanal");
        case 1:
            return tr("Kanalnamen");
        default:
            return QString();
        }
    }

    QString ledname() const;
    void setLedname(const QString& ledname);
    unsigned int channel() const;
    void setChannel(unsigned int channel);
  private:
    QString m_ledname;
    unsigned int m_channel;
};

#endif // ACTORLEDNAMESERVICEPROVIDER_H
