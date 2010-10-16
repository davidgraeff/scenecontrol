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

#ifndef CINAMESTATETRACKER_H
#define CINAMESTATETRACKER_H
#include "abstractstatetracker.h"
#include "media/mediacmds.h"

class OrgFreedesktopMediaPlayerInterface;
class CinemaStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url);
    Q_PROPERTY(int position READ position);
    Q_PROPERTY(int volume READ volume);
    Q_PROPERTY(int state READ state);

public:
    CinemaStateTracker(QObject* parent = 0);
    virtual ~CinemaStateTracker();
    QString url();
    int position();
    int volume();
    EnumMediaState state();
    void setState(EnumMediaState s) { m_cachedstate = s; }
private:
    EnumMediaState m_cachedstate;
    QString m_cachedurl;
    int m_cachedposition;
    int m_cachedvolume;
};

#endif // MPRISSTATETRACKER_H
