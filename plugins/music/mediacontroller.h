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

#ifndef PLAYLISTMEDIACONTROLLER_H
#define PLAYLISTMEDIACONTROLLER_H
#include <QAbstractListModel>
#include <QStringList>
#include <QUuid>
#include <QTimer>

#include <QTcpSocket>
#include <qprocess.h>
#include "statetracker/mediastatetracker.h"

class ExecuteWithBase;
class myPluginExecute;
class ActorPlaylistServer;
class MusicVolumeStateTracker;
class PAStateTracker;
class VolumeStateTracker;
class AbstractStateTracker;
class MediaStateTracker;
class AbstractServiceProvider;

class MediaController : public QObject
{
    Q_OBJECT
public:
    MediaController(myPluginExecute* plugin);
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
    ActorPlaylistServer* playlist();

    /**
     * Set active playlist. Resync object
     */
    void setPlaylist(ActorPlaylistServer* current);
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

    MediaStateTracker::EnumMediaState state();

    void setVolume(qreal newvol, bool relative = false);
    qreal volume() const;

private:
    myPluginExecute* m_plugin;
    QMap<QByteArray, PAStateTracker*> m_paStateTrackers;
    QList<ActorPlaylistServer*> m_items;
    ActorPlaylistServer* m_current;
    ActorPlaylistServer* m_favourite;
    MediaStateTracker *m_mediaStateTracker;
    MusicVolumeStateTracker* m_volumestateTracker;
    MediaStateTracker::EnumMediaState m_mediastate;
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
    void removedPlaylist(QObject* p);
public Q_SLOTS:
    void addedPlaylist(ActorPlaylistServer* playlist);
Q_SIGNALS:
    void pluginobjectChanged(ExecuteWithBase*);
    void stateChanged(AbstractStateTracker*);
};

#endif // PLAYLISTMEDIACONTROLLER_H
