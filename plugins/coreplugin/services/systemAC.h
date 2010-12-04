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
#include "shared/abstractserviceprovider.h"

class AbstractPlugin;

class ActorSystem : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(ActorSystem::actionEnum action READ action WRITE setAction);
public:
    ActorSystem(QObject* parent=0);
	virtual QString service_name(){return tr("Serveraktion");}
	virtual QString service_desc(){return tr("Gibt ein Kommando an den Raumkontrollserver");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Neustart");
            case 1:
                return tr("Beenden");
            case 2:
                return tr("Resynchronisieren");
            default:
                return tr("Kommando");
            }
        default:
            return QString();
        }
    }
    enum actionEnum
    {
      RestartSystem,
	  QuitSystem,
      ResyncSystem
    };
    Q_ENUMS(actionEnum);
    ActorSystem::actionEnum action() const;
    void setAction(ActorSystem::actionEnum value);
private:
	ActorSystem::actionEnum m_action;
};
