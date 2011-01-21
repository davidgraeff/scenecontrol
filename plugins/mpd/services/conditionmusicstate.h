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

#ifndef ConditionMusicState_h
#define ConditionMusicState_h
#include <shared/abstractserviceprovider.h>
#include <statetracker/mediastatetracker.h>

class ConditionMusicState : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(MediaStateTracker::EnumMediaState value READ value WRITE setValue);
    Q_ENUMS(MediaStateTracker::EnumMediaState);
public:
    ConditionMusicState(QObject* parent = 0);
	virtual QString service_name(){return tr("Musikstatusbedindung");}
	virtual QString service_desc(){return tr("Bedindung, dass der aktuelle Abspielstatus dem gesetzten entspricht");}

    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Abspielend");
            case 1:
                return tr("Pausiert");
            case 2:
                return tr("Angehalten");
            default:
                return tr("Status");
            }
        default:
            return QString();
        }
    }
    MediaStateTracker::EnumMediaState value() const;
    void setValue(MediaStateTracker::EnumMediaState value);
private:
    MediaStateTracker::EnumMediaState m_value;
};

#endif // ConditionMusicState_h
