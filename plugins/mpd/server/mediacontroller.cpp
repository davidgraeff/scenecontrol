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

#include "mediacontroller.h"
#include <QSettings>
#include <limits>
#include <stdio.h>
#include "statetracker/volumestatetracker.h"
#include "plugin_server.h"
#include <shared/abstractplugin.h>
#include <statetracker/playliststatetracker.h>
#include <shared/qjson/qobjecthelper.h>

#define MPD_PORT 6600
#define MPD_SERVER "192.168.1.2"

MediaController::MediaController(myPluginExecute* plugin) : m_plugin(plugin), m_mediastate(MediaStateTracker::StopState), m_currenttime(0), m_totaltime(0), m_terminate(false)
{
    m_mediaStateTracker = new MediaStateTracker(this);
    m_volumestateTracker = new MusicVolumeStateTracker(this);
    m_fakepos.setInterval(1000);
    connect(&m_fakepos, SIGNAL(timeout()),SLOT(updatefakepos()));

    m_mpdstatus = new QTcpSocket();
    connect (m_mpdstatus, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(sloterror(QAbstractSocket::SocketError)));
    connect (m_mpdstatus, SIGNAL(disconnected()),SLOT(slotdisconnected()));
    connect (m_mpdstatus, SIGNAL(connected()),SLOT(slotconnected()));
    connect (m_mpdstatus, SIGNAL(readyRead()),SLOT(slotreadyRead()));
    m_mpdstatus->connectToHost(QLatin1String(MPD_SERVER),MPD_PORT);

    m_mpdcmd = new QTcpSocket();
    connect (m_mpdcmd, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(sloterror2(QAbstractSocket::SocketError)));
    connect (m_mpdcmd, SIGNAL(disconnected()),SLOT(slotdisconnected2()));
    connect (m_mpdcmd, SIGNAL(connected()),SLOT(slotconnected2()));
    connect (m_mpdcmd, SIGNAL(readyRead()),SLOT(slotreadyRead2()));
    m_mpdcmd->connectToHost(QLatin1String(MPD_SERVER),MPD_PORT);

}

MediaController::~MediaController()
{
    m_terminate = true;
    m_mpdstatus->disconnectFromHost();
    qDeleteAll(m_playlists);
}

void MediaController::slotconnected() {
    if (m_terminate) return;
    needstatus = StatusNeed;
    needPlaylists = StatusNeed;
}

void MediaController::slotdisconnected()
{
    if (m_terminate) return;
    qWarning()<<"Connection to mpd lost. Try reconnect (status channel)";
    m_mpdstatus->connectToHost(QLatin1String(MPD_SERVER),MPD_PORT);
}

void MediaController::sloterror(QAbstractSocket::SocketError)
{
    if (m_terminate) return;
    qWarning()<<"mpd communication error (status channel)";
}

void MediaController::slotconnected2() {
    if (m_terminate) return;
    m_mpdchannelfree = false;
}

void MediaController::slotdisconnected2()
{
    if (m_terminate) return;
    qWarning()<<"Connection to mpd lost. Try reconnect (cmd channel)";
    m_mpdcmd->connectToHost(QLatin1String(MPD_SERVER),MPD_PORT);
}

void MediaController::sloterror2(QAbstractSocket::SocketError)
{
    if (m_terminate) return;
    qWarning()<<"mpd communication error (cmd channel)";
}

void MediaController::slotreadyRead2()
{
    while (m_mpdcmd->bytesAvailable())
    {
        QByteArray line = m_mpdcmd->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');

        if (args[0] == "OK") {
            if (m_commandqueue.size()) {
                m_mpdcmd->write(m_commandqueue.takeFirst());
            } else
                m_mpdchannelfree = true;
        }
    }
}

void MediaController::addToCommandQueue(const QByteArray& cmd) {
    m_commandqueue.append(cmd);
    if (m_mpdchannelfree) {
        m_mpdchannelfree = false;
        m_mpdcmd->write(m_commandqueue.takeFirst());
    }
}

void MediaController::slotreadyRead()
{
    bool needsync = false;
    const MediaStateTracker::EnumMediaState before = m_mediastate;
    while (m_mpdstatus->bytesAvailable())
    {
        QByteArray line = m_mpdstatus->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');

        if (args[0] == "OK") {
            // Fetching OK
            if (needstatus==StatusFetching) {
                needstatus=StatusNoNeed;
            }
            else if (needPlaylists==StatusFetching) {
                needPlaylists=StatusNoNeed;
                checkPlaylists();
            }
            // Need
            if (needstatus==StatusNeed) {
                m_mpdstatus->write("status\n");
                needstatus = StatusFetching;
            } else if (needPlaylists==StatusNeed) {
                m_mpdstatus->write("listplaylists\n");
                needPlaylists = StatusFetching;
            } else {
                m_mpdstatus->write("idle\n");
            }
            continue;
        }

        if (args[1]=="state:" && args.size()==2) {
            if (args[2]=="play") {
                m_mediastate = MediaStateTracker::PlayState;
                m_mediaStateTracker->setState(m_mediastate);
                m_fakepos.start();
            }
            else if (args[1]=="pause") {
                m_mediastate = MediaStateTracker::PauseState;
                m_mediaStateTracker->setState(m_mediastate);
                m_fakepos.stop();
            }
            else if (args[1]=="stopped") {
                m_mediastate = MediaStateTracker::StopState;
                m_mediaStateTracker->setState(m_mediastate);
                m_fakepos.stop();
            }
        } else if (args[0] == "volume:" && args.size()==2) {
            m_volume = args[1].toInt();
            m_volumestateTracker->setVolume(m_volume);
            emit stateChanged(m_volumestateTracker);
        } else if (args[0] == "time:" && args.size()==2) {
            QList<QByteArray> l = args[1].split(':');
            if (l.size()==1) l.append(0);
            const int current = l[0].toInt();
            const int total = l[1].toInt();
            m_mediaStateTracker->setPosition(current);
            m_mediaStateTracker->setTotal(total);
            if (m_totaltime!=total || m_currenttime!=current) needsync = true;
            m_currenttime = current;
            m_totaltime = total;
        } else if (args[0] == "song:" && args.size()==2) {
            const int trackno = args[1].toInt();
            if (m_totaltime!=trackno) needsync = true;
            m_mediaStateTracker->setTrack(trackno);
        } else if (args[0] == "playlist:" && args.size()==2 && needPlaylists==StatusFetching) {
            args.removeAt(0);
            QByteArray filename;
            foreach (QByteArray s, args) filename.append(s);
            m_tempplaylists.append(QString::fromUtf8(filename));
        } else if (args[0] == "Last-Modified:" && args.size()==2) {
            m_tempplaylists.append(QString::fromAscii(args[1]));
        } else if (args[0] == "changed:" && args.size()==2) {
            if (args[1] == "stored_playlist")
                needPlaylists = StatusNeed;
            else if (args[1] == "player")
                needstatus = StatusNeed;
            else if (args[1] == "mixer")
                needstatus = StatusNeed;
        }
    }

    if (before!=m_mediastate) needsync = true;
    if (needsync) {
        emit stateChanged(m_mediaStateTracker);
    }
}

QList<AbstractStateTracker*> MediaController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    l.append(m_mediaStateTracker);
    l.append(m_volumestateTracker);
    foreach(PlaylistStateTracker* p, m_playlists) l.append(p);
    return l;
}

void MediaController::nextPlaylist()
{
    int i = indexOfPlaylist(m_currentplaylist);
    if (i == -1) return;
    i += 1;
    if (i>=m_playlists.size()) i = 0;
    setPlaylistByIndex(i);
}

void MediaController::previousPlaylist()
{
    int i = indexOfPlaylist(m_currentplaylist);
    if (i == -1) return;
    i -= 1;
    if (i<0) i = m_playlists.size()-1;
    setPlaylistByIndex(i);
}

void MediaController::setPlaylistByIndex ( int index )
{
    PlaylistStateTracker* s = m_playlists.value(index, 0);
    if (!s) return;
    setPlaylist ( s->name() );
}

void MediaController::setPlaylist(const QString& name)
{
    if (name.isEmpty() || m_currentplaylist == name) return;
    m_currentplaylist = name;
    m_mediaStateTracker->setPlaylistid(name);
    emit stateChanged(m_mediaStateTracker);
	addToCommandQueue("clear\n");
    addToCommandQueue("load "+ name.toUtf8() + "\n");
}

QString MediaController::currentplaylist()
{
    return m_currentplaylist;
}

void MediaController::setVolume ( int newvol, bool relative )
{
    if (relative) m_volume = qBound<int>(0,m_volume + newvol,100);
    else m_volume = newvol;
    m_volumestateTracker->setVolume(m_volume);
    emit stateChanged(m_volumestateTracker);
    addToCommandQueue("setvol "+QByteArray::number(m_volume) + "\n");
}

int MediaController::volume() const
{
    return m_volume;
}

void MediaController::next()
{
    addToCommandQueue("next\n");
}

void MediaController::previous()
{
    addToCommandQueue("previous\n");
}

void MediaController::stop()
{
    addToCommandQueue("stop\n");
}

void MediaController::pause()
{
    if (m_mediastate == MediaStateTracker::PlayState)
        addToCommandQueue("pause 1\n");
    else
        addToCommandQueue("pause 0\n");
}

void MediaController::play()
{
    addToCommandQueue("play\n");
}

void MediaController::setTrackPosition ( qint64 pos, bool relative )
{
    // Don't allow seek if total runtime is not known
    // Instead ask media process again for the total runtime
    if (m_totaltime==0 && relative) {
        qWarning() << "Tried to seek relative without knowing the total time";
        return;
    }
    //qDebug() << __FUNCTION__ << m_currenttime << m_totaltime;

    if (relative) m_currenttime += pos;
    else m_currenttime = pos;

    addToCommandQueue("seek " + QByteArray::number(m_mediaStateTracker->track()) + " " + QByteArray::number(m_currenttime) + "\n");
    m_mediaStateTracker->setPosition(m_currenttime);
    emit stateChanged(m_mediaStateTracker);
}

qint64 MediaController::getTrackPosition()
{
    return m_currenttime;
}

qint64 MediaController::getTrackDuration()
{
    return m_totaltime;
}

MediaStateTracker::EnumMediaState MediaController::state()
{
    return m_mediastate;
}

int MediaController::count()
{
    return m_playlists.count();
}

void MediaController::updatefakepos() {
    m_currenttime+=1000;
}

int MediaController::indexOfPlaylist(const QString& name) {
    for (int i=0;i<m_playlists.size();++i) {
        if (m_playlists[i]->name() == name) return i;
    }
    return -1;
}

void MediaController::setCurrentTrack(int trackno) {
    addToCommandQueue("seek " + QByteArray::number(trackno) + " 0\n");
}

void MediaController::checkPlaylists() {
    if (m_tempplaylists.isEmpty()) return;

    QVector<int> vorhanden;
    vorhanden.resize(m_playlists.size());

    // add new
    for (int i=0;i<m_tempplaylists.size()-1;i+=2) {
        int index = indexOfPlaylist(m_tempplaylists[i]);
        if (index!=-1) {
            vorhanden[index] = 1;
            continue;
        }
        PlaylistStateTracker* statetracker = new PlaylistStateTracker();
        statetracker->setName(m_tempplaylists[i]);
        if (i+1<m_tempplaylists.size())
            statetracker->setLastModified(m_tempplaylists[i+1]);
        m_playlists.append(statetracker);
        emit stateChanged(statetracker);
    }

    //remove old
    for (int i=vorhanden.size()-1;i>=0;--i) {
        if (vorhanden[i] == true) continue;
        m_playlists[i]->setRemoved(true);
        emit stateChanged(m_playlists[i]);
        m_playlists.takeAt(i)->deleteLater();
    }

    m_tempplaylists.clear();
}

