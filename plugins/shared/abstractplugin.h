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

#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H
#include <QObject>
#include <QStringList>
#include "abstractstatetracker.h"
#include "abstractserviceprovider.h"

class AbstractPlugin
{
public:
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    /**
     * Return all Actions, Conditions, Events and StateTracker
     */
    virtual QStringList registerServices() const = 0;
	virtual QStringList registerStateTracker() const = 0;
    virtual AbstractStateTracker* createStateTracker(const QString& id) = 0;
	virtual AbstractServiceProvider* createServiceProvider(const QString& id) = 0;
};

#endif // ABSTRACTPLUGIN_H
