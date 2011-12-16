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
#include <QUrl>

class AbstractPlugin;
class MediaController : public QObject
{
    Q_OBJECT
public:
	enum MediaState {
		NothingLoaded,
		StopState,
		PauseState,
		PlayState
	};
    MediaController(AbstractPlugin* plugin);
    ~MediaController();
	const QString host() const { return m_host; }
	int port() const { return m_port; }
	int isConnected() const { return m_mpdstatus->isOpen()&&m_mpdcmd->isOpen(); }
	
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

    // state
    MediaState state();

    // volume
    void setVolume(int newvol, bool relative = false);
	
	/**
	 * Return volume. Range 0-100
	 */
    int volume() const;

    void dumpMediaInfo();
    void connectToMpd(const QString& host, int port);
private:
    AbstractPlugin* m_plugin;
    QString m_host;
    int m_port;
    bool m_terminate;

    // command queue
    void addToCommandQueue(const QByteArray& cmd);
    QList<QByteArray> m_commandqueue;
    bool m_mpdchannelfree;

    // build playlist statetrackers
    QList< QString > m_playlists;
	QList< QString > m_tempplaylists;
    QString m_playlistid;

    int indexOfPlaylist(const QString& name);

    // current media state
    MediaState m_mediastate;
    qint64 m_currenttime;
    qint64 m_totaltime;
	int m_track;
    int m_volume;
    QTimer m_fakepos;
    QString m_tracktitle;
    QString m_trackfile;
    QString m_trackname;

    // sockets
    QTcpSocket* m_mpdstatus;
    QTcpSocket* m_mpdcmd;
    QTimer m_reconnect;

    // commands for status socket
    enum StatusEnum {
        StatusNoNeed,
        StatusNeed,
        StatusFetching
    };
    StatusEnum m_needstatus;
    StatusEnum m_needPlaylists;
    StatusEnum m_needCurrentSong;

    // dump media info
    void saveMediaInfo();
	void checkPlaylists();
private Q_SLOTS:
    void writeCommandQueueItem();
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected();
    void sloterror(QAbstractSocket::SocketError e);
    void slotreadyRead2();
    void slotconnected2();
    void slotdisconnected2();
    void sloterror2(QAbstractSocket::SocketError e);
    void updatefakepos();
    void reconnectTimeout();
Q_SIGNALS:
    void volumeChanged(double);
	void playlistsChanged(const QString& playlist, int position);
	void playlistChanged(const QString& playlist_id);
	void trackChanged(const QString& filename, const QString& trackname, int track, uint position_in_ms, uint total_in_ms, int state);
	void stateChanged(MediaController* client);
};
