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
    Q_ENUMS(ProjectorControl);
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
	virtual QString service_name(){return tr("Projektorsteuerung");}
	virtual QString service_desc(){return tr("Steuert den Z700 Projektor");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Einschalten");
            case 1:
                return tr("Ausschalten");
            case 2:
                return tr("Video aus");
            case 3:
                return tr("Video an");
            case 4:
                return tr("Lampe normal");
            case 5:
                return tr("Lampe eco");
            default:
                return tr("Kommando");
            }
        default:
            return QString();
        }
    }
    void setCmd(ActorProjector::ProjectorControl v);
    ActorProjector::ProjectorControl cmd();
private:
    ActorProjector::ProjectorControl m_cmd;
};

#endif // ACTORPROJECTORSERVICEPROVIDER_H
