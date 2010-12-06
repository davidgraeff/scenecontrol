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
#include <shared/abstractstatetracker.h>

class AmbienceVideoStateTracker : public AbstractStateTracker
{
	Q_OBJECT
	Q_PROPERTY(QString filename READ filename WRITE setFilename);
	Q_PROPERTY(EnumAmbienceState state READ state WRITE setState);
	Q_ENUMS(EnumAmbienceState);
public:
    enum EnumAmbienceState
    {
        PlayState,
        PauseState,
        CloseState,
	ErrorCloseState
    };
	AmbienceVideoStateTracker(QObject* parent = 0) : AbstractStateTracker(parent) {}
	QString filename() { return m_filename; }
	void setFilename(QString filename) {m_filename = filename;}
	EnumAmbienceState state() { return m_state; }
	void setState(EnumAmbienceState state) {m_state = state;}
private:
	QString m_filename;
	EnumAmbienceState m_state;
};
