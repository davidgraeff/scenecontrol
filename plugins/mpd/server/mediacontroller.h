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
#include <QStringList>
#include <QTimer>
#include <QTcpSocket>
#include <statetracker/mediastatetracker.h>

class MusicVolumeStateTracker;
class PlaylistStateTracker;
class AbstractStateTracker;
class ExecuteWithBase;
class myPluginExecute;
class AbstractServiceProvider;

class MediaController : public QObject
{
    Q_OBJECT
public:
    MediaController(myPluginExecute* plugin);
    ~MediaController();
    QList<AbstractStateTracker*> getStateTracker();

    /**
     * Return current active playlist or an empty string if no valid playlist is set
     */
    QString currentplaylist();

    /**
     * Set active playlist. Resync object
     */
    void setPlaylist(const QString& name);
    /**
     * Set active playlist by using the index. Resync object
     */
    void setPlaylistByIndex(int index);
    /**
     * Set track no of current playlist
     */
    void setCurrentTrack(int trackno);

    void nextPlaylist();
    void previousPlaylist();
    void next();
    void previous();
    void stop();
    void pause();
    void play();

    /**
     * Set track position. Does nothing if no active playlist. Resync object
     */
    void setTrackPosition(qint64 pos, bool relative=false);
    qint64 getTrackPosition();
    qint64 getTrackDuration();

    MediaStateTracker::EnumMediaState state();

    void setVolume(int newvol, bool relative = false);
    int volume() const;

    int count();
    void checkPlaylists();

    void addToCommandQueue(const QByteArray& cmd);
private:
    QList<QByteArray> m_commandqueue;
    QList< QString > m_tempplaylists;
    myPluginExecute* m_plugin;
    QList<PlaylistStateTracker*> m_playlists;
    QString m_currentplaylist;
    MediaStateTracker *m_mediaStateTracker;
    MusicVolumeStateTracker* m_volumestateTracker;
    MediaStateTracker::EnumMediaState m_mediastate;
    qint64 m_currenttime;
    qint64 m_totaltime;
    int m_volume;
    QTimer m_fakepos;
    QTcpSocket* m_mpdstatus;
	QTcpSocket* m_mpdcmd;
    bool m_terminate;
    enum StatusEnum {
        StatusNoNeed,
        StatusNeed,
        StatusFetching
    };
    StatusEnum needstatus;
    StatusEnum needPlaylists;
    bool m_mpdchannelfree;
    int indexOfPlaylist(const QString& name);
private Q_SLOTS:
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected();
    void sloterror(QAbstractSocket::SocketError e);
	void slotreadyRead2();
	void slotconnected2();
	void slotdisconnected2();
	void sloterror2(QAbstractSocket::SocketError e);
	void updatefakepos();
Q_SIGNALS:
    void stateChanged(AbstractStateTracker*);
};
