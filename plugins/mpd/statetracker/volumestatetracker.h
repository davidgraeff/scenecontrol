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

#ifndef MusicVolumeStateTracker_h
#define MusicVolumeStateTracker_h
#include <shared/abstractstatetracker.h>

class MusicVolumeStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume);
public:
    MusicVolumeStateTracker(QObject* parent = 0) : AbstractStateTracker(parent) {}
    int volume() {
        return m_volume;
    }
    void setVolume(int volume) {
        m_volume = volume;
    }
private:
	int m_volume;
};
#endif //MusicVolumeStateTracker_h