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

#ifndef MediaStateTracker_h
#define MediaStateTracker_h
#include "abstractstatetracker.h"
#include "media/mediacmds.h"

class MediaStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString playlistid READ playlistid);
    Q_PROPERTY(qint64 position READ position);
    Q_PROPERTY(qint64 total READ total);
    Q_PROPERTY(int track READ track);
    Q_PROPERTY(int state READ state);
public:
    MediaStateTracker(QObject* parent = 0);
    QString playlistid() const;
    qint64 position() const;
    qint64 total() const;
    int state();
    int track() const;

};

#endif // MediaStateTracker_h
