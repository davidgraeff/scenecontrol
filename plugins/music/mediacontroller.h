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

#ifndef PLAYLISTCONTROLLER_H
#define PLAYLISTCONTROLLER_H
#include <QAbstractListModel>
#include <QStringList>
#include <QUuid>
#include <QTimer>
#include "mediacmds.h"
#include <QTcpSocket>
#include <qprocess.h>

class PAStateTracker;
class VolumeStateTracker;
class AbstractStateTracker;
class MediaStateTracker;
class AbstractServiceProvider;

class Playlist;

class MediaController : public QObject
{
  Q_OBJECT
  public:
    MediaController();
    ~MediaController();
    int count();
    QList<AbstractStateTracker*> getStateTracker();
    
    /** 
     * Do nothing if favourite playlist is already added (and activated)
     * else create a new playlist with the name "favourite"
     */
    void activateFavourite();
    
    /**
     * Return current active playlist or 0 if no valid playlist set
     */
    Playlist* playlist();
    
    /**
     * Set active playlist. Resync object
     */
    void setPlaylist(Playlist* current);
    /**
     * Set active playlist by using the id. Resync object
     */
    void setPlaylistByID(const QString &id);
    /**
     * Set active playlist by using the index. Resync object
     */
    void setPlaylistByIndex(int index);

    void nextPlaylist();
    void previousPlaylist();
    void next();
    void previous();
    void stop();
    void pause();
    void play();

	void setPAMute(const QByteArray sink, bool mute);
	void togglePAMute(const QByteArray sink);
	void setPAVolume(const QByteArray sink, double volume, bool relative = false);

    /**
     * Set track position. Does nothing if no active playlist. Resync object
     */
    void setTrackPosition(qint64 pos, bool relative=false);
    qint64 getTrackPosition();
    qint64 getTrackDuration();

    EnumMediaState state();

    void setVolume(qreal newvol, bool relative = false);
    qreal volume() const;
    
  private:
 	QMap<QByteArray, PAStateTracker*> m_paStateTrackers;
    QList<Playlist*> m_items;
    Playlist* m_current;
    Playlist* m_favourite;
    MediaStateTracker *m_mediaStateTracker;
    VolumeStateTracker* m_volumestateTracker;
	EnumMediaState m_mediastate;
	qint64 m_currenttime;
	qint64 m_totaltime;
	qreal m_volume;
	bool m_requestTotal;
	QProcess* m_playerprocess;
	QTimer m_fakepos;
private Q_SLOTS:
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected(int);
    void sloterror(QProcess::ProcessError e);
    void updatefakepos();
    void readyReadStandardError() ;
  public Q_SLOTS:
    void addedProvider(AbstractServiceProvider* provider);
    void removedProvider(AbstractServiceProvider* provider);
};

#endif // PLAYLISTCONTROLLER_H
