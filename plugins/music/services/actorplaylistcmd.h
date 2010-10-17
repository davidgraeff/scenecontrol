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

#ifndef ACTORPLAYLISTCMDSERVICEPROVIDER_H
#define ACTORPLAYLISTCMDSERVICEPROVIDER_H
#include <shared/abstractserviceprovider.h>

class ActorPlaylistCmd : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(ActorPlaylistCmd::EnumMediaCmd cmd READ cmd WRITE setCmd);
public:
enum EnumMediaCmd
{
    PlayCmd,
    StopCmd,
    PauseCmd,
    NextCmd,
    PrevCmd,
    NextPlaylistCmd,
    PrevPlaylistCmd,
    InfoCmd,
    AspectRatioCmd,
    NextSubtitleCmd,
    NextLanguageCmd,
    NavigationUpCmd,
    NavigationDownCmd,
    NavigationLeftCmd,
    NavigationRightCmd,
    NavigationBackCmd,
    NavigationOKCmd,
    NavigationHomeCmd,
    NavigationCloseCmd,
    NavigationContextMenuCmd,
    FastForwardCmd,
    FastRewindCmd
};
Q_ENUMS(EnumMediaCmd);

    ActorPlaylistCmd(QObject* parent = 0);
    virtual ProvidedTypes providedtypes() {
        return ActionType;
    }
    ActorPlaylistCmd::EnumMediaCmd cmd() const ;
    void setCmd(ActorPlaylistCmd::EnumMediaCmd value) ;
private:
    ActorPlaylistCmd::EnumMediaCmd m_cmd;
};

#endif // ACTORPLAYLISTCMDSERVICEPROVIDER_H
