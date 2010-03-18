/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef VOLUMESTATETRACKER_H
#define VOLUMESTATETRACKER_H
#include "abstractstatetracker.h"

#define EVENTVOLUME_ID 1
#define MEDIAVOLUME_ID 2
#define CINEMAVOLUME_ID 3

class VolumeStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(int track READ track WRITE setTrack);
    Q_PROPERTY(int volume READ volume WRITE setVolume);
public:
    VolumeStateTracker(QObject* parent = 0);
    int volume() const;
    int track() const;
	void setTrack(int track) { m_track = track; }
	void setVolume(int vol) { m_volume = vol; }
private:
	int m_track;
	int m_volume;
};

#endif // VOLUMESTATETRACKER_H
