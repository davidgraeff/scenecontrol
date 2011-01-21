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

#ifndef CinemaStateTracker_h
#define CinemaStateTracker_h
#include <shared/abstractstatetracker.h>

class CinemaStateTracker : public AbstractStateTracker
{
	Q_OBJECT
	Q_PROPERTY(QString url READ url WRITE setUrl);
	Q_PROPERTY(int state READ state WRITE setState);
public:
	CinemaStateTracker(QObject* parent = 0) : AbstractStateTracker(parent),m_volume(0),m_state(0) {}
	QString url() { return m_url; }
	void setUrl(QString url) {m_url = url;}
	int state() { return m_state; }
	void setState(int state) {m_state = state;}
private:
	QString m_url;
	int m_volume;
	int m_state;
};
#endif //CinemaStateTracker_h