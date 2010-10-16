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

#include "cinamestatetracker.h"
#include <RoomControlServer.h>

CinemaStateTracker::CinemaStateTracker(QObject* parent)
        : AbstractStateTracker(parent),
        m_cachedstate(StopState), m_cachedposition(0), m_cachedvolume(0)
{
}

CinemaStateTracker::~CinemaStateTracker()
{
}


QString CinemaStateTracker::url()
{
    return m_cachedurl;
    //interface->GetMetadata().value().value(QLatin1String("location")).toString();
}

int CinemaStateTracker::position()
{
    return m_cachedposition;
}

int CinemaStateTracker::volume()
{
    return m_cachedvolume;
}

EnumMediaState CinemaStateTracker::state()
{
    return m_cachedstate;
}
