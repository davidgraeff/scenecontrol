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

class EventSystem : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(EventSystem::systemEnum system READ system WRITE setSystem)
	Q_ENUMS(systemEnum);
public:
  	enum systemEnum
	{
		ServerStarted,
		ServerGoingToStop
	};
	
    EventSystem(QObject* parent = 0);
	virtual QString service_name(){return tr("Serverereignis");}
	virtual QString service_desc(){return tr("Wird von Serverereignissen ausgelöst wie z.B. dem Start des Servers");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Startet");
            case 1:
                return tr("Wird beendet");
            default:
                return tr("Serverereignis");
            }
        default:
            return QString();
        }
    }
    EventSystem::systemEnum system() const ;
    void setSystem( EventSystem::systemEnum s ) ;
private:
    EventSystem::systemEnum m_system;
};
