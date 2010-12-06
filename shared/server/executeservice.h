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
#include <QObject>
#include <QStringList>
#include <shared/abstractserviceprovider.h>
#include "executeWithBase.h"

class ExecuteService : public ExecuteWithBase
{
    Q_OBJECT
public:
    ExecuteService(AbstractServiceProvider* base, QObject* parent = 0);
    virtual bool checkcondition() = 0;
    virtual void execute() = 0;
    virtual void dataUpdate() = 0;
	virtual void nameUpdate() {}
Q_SIGNALS:
    void trigger();
};
