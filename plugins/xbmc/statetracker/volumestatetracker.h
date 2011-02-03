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

#ifndef CinemaVolumeStateTracker_h
#define CinemaVolumeStateTracker_h
#include <shared/abstractstatetracker.h>

class CinemaVolumeStateTracker : public AbstractStateTracker
{
	Q_OBJECT
	Q_PROPERTY(double volume READ volume WRITE setVolume);
public:
	CinemaVolumeStateTracker(QObject* parent = 0) : AbstractStateTracker(parent),m_volume(0) {}
	double volume() { return m_volume; }
	void setVolume(double volume) {m_volume = volume;}
private:
	double m_volume;
};
#endif //CinemaVolumeStateTracker_h