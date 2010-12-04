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
    Q_PROPERTY(EnumMediaCmd cmd READ cmd WRITE setCmd);
    Q_ENUMS(EnumMediaCmd);
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
        FastForwardCmd,
        FastRewindCmd
    };

    ActorPlaylistCmd(QObject* parent = 0);
    virtual QString service_name() {
        return tr("Abspiellistenkommando");
    }
    virtual QString service_desc() {
        return tr("Schickt ein Kommando an die ausgewählte Abspielliste");
    }

    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Abspielen");
            case 1:
                return tr("Stoppen");
            case 2:
                return tr("Pausieren");
            case 3:
                return tr("Nächster Track");
            case 4:
                return tr("Vorheriger Track");
            case 5:
                return tr("Nächste Abspielliste");
            case 6:
                return tr("Vorherige Abspielliste");
            case 7:
                return tr("Info anzeigen");
            case 8:
                return tr("Nächste Aspektratio");
            case 9:
                return tr("Nächste Untertitelsprache");
            case 10:
                return tr("Nächste Sprache");
            case 11:
                return tr("Vorspulen");
            case 12:
                return tr("Zurückspulen");
            default:
                return tr("Kommando");
            }
        default:
            return QString();
        }
    }
    ActorPlaylistCmd::EnumMediaCmd cmd() const ;
    void setCmd(ActorPlaylistCmd::EnumMediaCmd value) ;
private:
    ActorPlaylistCmd::EnumMediaCmd m_cmd;
};

#endif // ACTORPLAYLISTCMDSERVICEPROVIDER_H
