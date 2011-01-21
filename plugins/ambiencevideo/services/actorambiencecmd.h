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

#pragma once
#include <shared/abstractserviceprovider.h>

class ActorAmbienceCmd : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(EnumAmbienceCmd cmd READ cmd WRITE setCmd)
    Q_ENUMS(EnumAmbienceCmd);
    Q_PROPERTY(int restoretime READ restoretime WRITE setRestoretime)
    Q_CLASSINFO("restoretime_min", "0");
    Q_CLASSINFO("restoretime_max", "1000");
public:
    enum EnumAmbienceCmd
    {
        CloseFullscreen,
        HideVideo,
        ScreenToggle,
        ScreenOn,
        ScreenOff,
        StopVideo
    };
    ActorAmbienceCmd(QObject* parent = 0);
    virtual QString service_name() {
        return tr("Ambience Video steuern");
    }
    virtual QString service_desc() {
        return tr("Setzt das angegebene Kommando ab");
    }

    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Vollbild temporär aufheben");
            case 1:
                return tr("Video temporär verstecken");
            case 2:
                return tr("Bildschirm ein/aus umschalten");
            case 3:
                return tr("Bildschirm einschalten");
            case 4:
                return tr("Bildschirm ausschalten");
            case 5:
                return tr("Video schließen");
            default:
                return tr("Kommando");
            }
        case 1:
			return tr("Wiederherstellungszeit in s");
        default:
            return QString();
        }
    }
    EnumAmbienceCmd cmd() const {
        return m_cmd;
    }

    void setCmd( EnumAmbienceCmd m ) {
        m_cmd = m;
    }
    
    int restoretime() const {
		return m_restoretime;
	}
	
	void setRestoretime( int v ) {
		m_restoretime = v;
	}
private:
    EnumAmbienceCmd m_cmd;
	int m_restoretime;
};
