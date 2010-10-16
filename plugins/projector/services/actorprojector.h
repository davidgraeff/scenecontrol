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

#ifndef ACTORPROJECTORSERVICEPROVIDER_H
#define ACTORPROJECTORSERVICEPROVIDER_H
#include <shared/abstractserviceprovider.h>

class ActorProjector : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(ActorProjector::ProjectorControl cmd READ cmd WRITE setCmd);
public:
    enum ProjectorControl {
        ProjectorOn,
        ProjectorOff,
        ProjectorVideoMute,
        ProjectorVideoUnMute,
        ProjectorLampNormal,
        ProjectorLampEco
    };
    ActorProjector(QObject* parent = 0);
    virtual ProvidedTypes providedtypes(){return ActionType;}
    void setCmd(ActorProjector::ProjectorControl v);
    ActorProjector::ProjectorControl cmd();
private:
    ActorProjector::ProjectorControl m_cmd;
};

#endif // ACTORPROJECTORSERVICEPROVIDER_H
