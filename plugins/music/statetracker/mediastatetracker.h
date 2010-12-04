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
#include <shared/abstractstatetracker.h>

class MediaStateTracker : public AbstractStateTracker
{
	Q_OBJECT
	Q_PROPERTY(QString playlistid READ playlistid WRITE setPlaylistid);
	Q_PROPERTY(qint64 position READ position WRITE setPosition);
	Q_PROPERTY(qint64 total READ total WRITE setTotal);
	Q_PROPERTY(int track READ track WRITE setTrack);
	Q_PROPERTY(EnumMediaState state READ state WRITE setState);
    Q_ENUMS(EnumMediaState);
public:
    enum EnumMediaState
    {
        PlayState,
        PauseState,
        StopState
    };

	MediaStateTracker(QObject* parent = 0) : AbstractStateTracker(parent) {}
	QString playlistid() { return m_playlistid; }
	void setPlaylistid(QString playlistid) {m_playlistid = playlistid;}
	qint64 position() { return m_position; }
	void setPosition(qint64 position) {m_position = position;}
	qint64 total() { return m_total; }
	void setTotal(qint64 total) {m_total = total;}
	int track() { return m_track; }
	void setTrack(int track) {m_track = track;}
	MediaStateTracker::EnumMediaState state() { return m_state; }
	void setState(MediaStateTracker::EnumMediaState state) {m_state = state;}
private:
	QString m_playlistid;
	qint64 m_position;
	qint64 m_total;
	int m_track;
	MediaStateTracker::EnumMediaState m_state;
};
#endif //MediaStateTracker_h