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

#ifndef ServiceWOLExecute_H
#define ServiceWOLExecute_H

#include "shared/server/executeservice.h"

class ActorWOL;
class ServiceWOLExecute : public ExecuteService
{
    Q_OBJECT
public:
    ServiceWOLExecute(ActorWOL* base, QObject* parent = 0);
    virtual void execute();
    virtual bool checkcondition();
    virtual void dataUpdate();
private:
};
#endif // ServiceWOLExecute_H
