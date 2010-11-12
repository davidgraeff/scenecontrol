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

#ifndef ActorPlaylistTrack_h
#define ActorPlaylistTrack_h
#include <shared/abstractserviceprovider.h>
#include <statetracker/mediastatetracker.h>
#include <QModelIndex>

class ActorPlaylistTrack : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString playlistid READ playlistid WRITE setPlaylistID);
    Q_CLASSINFO("playlistid_model", "PlaylistsModel")
    Q_CLASSINFO("playlistid_model_displaytype", "0");
    Q_CLASSINFO("playlistid_model_savetype", "32");
    Q_PROPERTY(int track READ track WRITE setTrack);
    Q_CLASSINFO("track_model", "TempPlaylistTracksModel")
    Q_CLASSINFO("track_model_displaytype", "0");
    Q_CLASSINFO("track_model_savetype", "32");
    Q_PROPERTY(MediaStateTracker::EnumMediaState state READ state WRITE setState);
public:
  Q_ENUMS(MediaStateTracker::EnumMediaState);
    ActorPlaylistTrack(QObject* parent = 0);
    virtual ProvidedTypes providedtypes(){return ActionType;}
    QString playlistid() const;
    void setPlaylistID(QString playlistid);
    int track() const;
    void setTrack(int value);
    MediaStateTracker::EnumMediaState state() const;
    void setState(MediaStateTracker::EnumMediaState value);
private:
    QString m_playlistid;
    int m_track;
    MediaStateTracker::EnumMediaState m_state;
};

#endif // ActorPlaylistTrack_h
