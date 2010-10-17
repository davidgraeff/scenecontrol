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

#ifndef ACTORCINEMASERVICEPROVIDER_H
#define ACTORCINEMASERVICEPROVIDER_H

#include "shared/abstractserviceprovider.h"
class ActorCinema : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(ActorCinema::CinemaCmds cmd READ cmd WRITE setCmd);
    Q_PROPERTY(QString url READ url WRITE setUrl);
public:
    enum CinemaCmds  {
    PlayCmd,PauseCmd,StopCmd,NextCmd,PrevCmd,InfoCmd,AspectRatioCmd,NextSubtitleCmd,NextLanguageCmd,NavigationBackCmd,NavigationHomeCmd,
    NavigationOKCmd,NavigationDownCmd,NavigationUpCmd,NavigationLeftCmd,NavigationRightCmd,NavigationCloseCmd,NavigationContextMenuCmd,
    FastForwardCmd,FastRewindCmd
    };
    Q_ENUMS(CinemaCmds);
    ActorCinema(QObject* parent = 0);
    virtual ProvidedTypes providedtypes() {
        return ActionType;
    }
    QString url() {
        return m_url;
    }
    void setUrl(QString url) {
        m_url = url;
    }
    ActorCinema::CinemaCmds cmd() const ;
    void setCmd(ActorCinema::CinemaCmds value) ;
private:
    ActorCinema::CinemaCmds m_cmd;
    QString m_url;
};
#endif // ACTORCINEMASERVICEPROVIDER_H
