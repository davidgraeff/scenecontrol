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

#include <QSettings>
#include <limits>
#include <stdio.h>
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>

#include "mpdController.h"
#include "abstractplugin.h"

MediaController::MediaController(AbstractPlugin* plugin) : m_plugin(plugin), m_terminate(false), m_mediastate(NothingLoaded), m_currenttime(0), m_totaltime(0), m_track(0), m_volume(0)
{
    m_fakepos.setInterval(1000);
    connect(&m_fakepos, SIGNAL(timeout()),SLOT(updatefakepos()));

    m_mpdstatus = new QTcpSocket();
    connect (m_mpdstatus, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(sloterror(QAbstractSocket::SocketError)));
    connect (m_mpdstatus, SIGNAL(disconnected()),SLOT(slotdisconnected()));
    connect (m_mpdstatus, SIGNAL(connected()),SLOT(slotconnected()));
    connect (m_mpdstatus, SIGNAL(readyRead()),SLOT(slotreadyRead()));

    m_mpdcmd = new QTcpSocket();
    connect (m_mpdcmd, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(sloterror2(QAbstractSocket::SocketError)));
    connect (m_mpdcmd, SIGNAL(disconnected()),SLOT(slotdisconnected2()));
    connect (m_mpdcmd, SIGNAL(connected()),SLOT(slotconnected2()));
    connect (m_mpdcmd, SIGNAL(readyRead()),SLOT(slotreadyRead2()));

    connect(&m_reconnect,SIGNAL(timeout()),SLOT(reconnectTimeout()));
    m_reconnect.setInterval(60000);
}

MediaController::~MediaController()
{
    m_terminate = true;
    m_mpdcmd->disconnectFromHost();
    m_mpdstatus->disconnectFromHost();
}

void MediaController::connectToMpd(const QString& host, int port)
{
    m_host = host;
    m_port = port;

    reconnectTimeout();
}

void MediaController::reconnectTimeout() {
    if (m_mpdstatus->state() == QTcpSocket::UnconnectedState)
        m_mpdstatus->connectToHost(m_host,m_port);
    if (m_mpdcmd->state() == QTcpSocket::UnconnectedState)
        m_mpdcmd->connectToHost(m_host,m_port);
}

void MediaController::slotconnected() {
    if (m_terminate) return;
    if (m_mpdstatus->state() != QTcpSocket::UnconnectedState && m_mpdcmd->state() != QTcpSocket::UnconnectedState) {
        emit stateChanged(this);
        m_reconnect.stop();
    }
    m_needCurrentSong = StatusNoNeed;
    m_needstatus = StatusNeed;
    m_needPlaylists = StatusNeed;
}

void MediaController::slotdisconnected()
{
    if (m_terminate) return;
    if (m_mpdstatus->state() != QTcpSocket::UnconnectedState) return;
	m_mediastate = NothingLoaded;
    emit stateChanged(this);
    qWarning() << m_plugin->pluginid() << "Connection lost. Try reconnect (status channel)";
    m_mpdstatus->connectToHost(m_host,m_port);
}

void MediaController::sloterror(QAbstractSocket::SocketError)
{
    if (m_terminate) return;
    //qWarning() << m_plugin->pluginid() << m_host << m_port << m_mpdstatus->errorString() << "status channel";
    m_reconnect.start();
}

void MediaController::slotconnected2() {
    if (m_terminate) return;
    m_mpdchannelfree = false;
    if (m_mpdstatus->state() != QTcpSocket::UnconnectedState && m_mpdcmd->state() != QTcpSocket::UnconnectedState) {
        m_reconnect.stop();
        emit stateChanged(this);
    }
}

void MediaController::slotdisconnected2()
{
    if (m_terminate) return;
    if (m_mpdcmd->state() != QTcpSocket::UnconnectedState) return;
    emit stateChanged(this);
    qWarning() << m_plugin->pluginid() << "Connection lost. Try reconnect (cmd channel)";
    m_mpdcmd->connectToHost(m_host,m_port);
}

void MediaController::sloterror2(QAbstractSocket::SocketError)
{
    if (m_terminate) return;
    //qWarning() << m_plugin->pluginid() << m_host << m_port << m_mpdcmd->errorString() << "cmd channel";
    m_reconnect.start();
}

void MediaController::slotreadyRead2()
{
    while (m_mpdcmd->bytesAvailable())
    {
        QByteArray line = m_mpdcmd->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');

        if (args[0] == "OK") {
            writeCommandQueueItem();
        } else {
            QTimer::singleShot(1000, this, SLOT(writeCommandQueueItem()));
        }
    }
}

void MediaController::writeCommandQueueItem()
{
    if (m_commandqueue.size()) {
        m_mpdcmd->write(m_commandqueue.takeFirst());
    } else
        m_mpdchannelfree = true;
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
    const MediaState before = m_mediastate;
    while (m_mpdstatus->bytesAvailable())
    {
        QByteArray line = m_mpdstatus->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');

        if (args[0] == "OK") {
            // Fetching OK
            if (m_needstatus==StatusFetching) {
                m_needstatus=StatusNoNeed;
            }
            else if (m_needPlaylists==StatusFetching) {
                m_needPlaylists=StatusNoNeed;
                checkPlaylists();
            } else if (m_needCurrentSong==StatusFetching) {
                m_needCurrentSong=StatusNoNeed;
                saveMediaInfo();
            }
            // Need
            if (m_needstatus==StatusNeed) {
                m_mpdstatus->write("status\n");
                m_needstatus = StatusFetching;
            } else if (m_needPlaylists==StatusNeed) {
                m_mpdstatus->write("listplaylists\n");
                m_needPlaylists = StatusFetching;
            } else if (m_needCurrentSong==StatusNeed) {
                m_mpdstatus->write("currentsong\n");
                m_needCurrentSong = StatusFetching;
            } else {
                m_mpdstatus->write("idle\n");
            }
            continue;
        }

        if (m_needCurrentSong == StatusFetching) {
            if (args[0]=="file:" && args.size()==2) {
                m_trackfile = QString::fromUtf8(args[1]);
            } else if (args[0]=="Title:" && args.size()==2) {
                m_tracktitle = QString::fromUtf8(args[1]);
            } else if (args[0]=="Name:" && args.size()==2) {
                m_trackname = QString::fromUtf8(args[1]);
            }
        } else if (m_needstatus == StatusFetching) {
            if (args[0]=="state:" && args.size()==2) {
                if (args[1]=="play") {
                    m_mediastate = PlayState;
                    m_fakepos.start();
                }
                else if (args[1]=="pause") {
                    m_mediastate = PauseState;
                    m_fakepos.stop();
                }
                else if (args[1]=="stopped") {
                    m_mediastate = StopState;
                    m_fakepos.stop();
                }
            } else if (args[0] == "volume:" && args.size()==2) {
                m_volume = args[1].toInt();
                emit volumeChanged(m_volume);
            } else if (args[0] == "time:" && args.size()==2) {
                QList<QByteArray> l = args[1].split(':');
				//TODO
//                 if (l.size()==1)
// 					changeProperty(0);
                const int current = l[0].toInt();
                const int total = l[1].toInt();
                if (m_totaltime!=total || m_currenttime!=current) needsync = true;
                m_currenttime = current;
                m_totaltime = total;
            } else if (args[0] == "song:" && args.size()==2) {
                const int track = args[1].toInt();
                if (m_totaltime!=track) needsync = true;
                m_track = track;
            }
        } else if (m_needPlaylists == StatusFetching) {
            if (args[0] == "playlist:" && args.size()==2 ) {
                args.removeAt(0);
                QByteArray filename;
                foreach (QByteArray s, args) filename.append(s);
                m_tempplaylists.append(QString::fromUtf8(filename));
            } else if (args[0] == "Last-Modified:" && args.size()==2) {
                m_tempplaylists.append(QString::fromAscii(args[1]));
            }
        } else if (args[0] == "changed:" && args.size()==2) {
            if (args[1] == "stored_playlist")
                m_needPlaylists = StatusNeed;
            else if (args[1] == "player")
                m_needstatus = StatusNeed;
            else if (args[1] == "mixer")
                m_needstatus = StatusNeed;
        }
    }

    if (before!=m_mediastate) needsync = true;
    if (needsync) {
        emit trackChanged(m_trackfile,m_trackname,m_track,m_currenttime,m_totaltime,m_mediastate);
    }
}

void MediaController::nextPlaylist()
{
    int i = indexOfPlaylist(m_playlistid);
    if (i == -1) {
        if (m_playlists.size()) i=0;
        else return;
    }
    i += 1;
    if (i>=m_playlists.size()) i = 0;
    setPlaylistByIndex(i);
}

void MediaController::previousPlaylist()
{
    int i = indexOfPlaylist(m_playlistid);
    if (i == -1) {
        if (m_playlists.size()) i=0;
        else return;
    }
    i -= 1;
    if (i<0) i = m_playlists.size()-1;
    setPlaylistByIndex(i);
}

void MediaController::setPlaylistByIndex ( int index )
{
    setPlaylist ( m_playlists.at(index) );
}

void MediaController::setPlaylist(const QString& name)
{
    if (name.isEmpty() || m_playlistid == name) return;
    m_playlistid = name;
    emit playlistChanged(name);
    addToCommandQueue("clear\n");
    addToCommandQueue("load "+ name.toUtf8() + "\n");
}

QString MediaController::currentplaylist()
{
    return m_playlistid;
}

void MediaController::setVolume ( int newvol, bool relative )
{
    if (relative) m_volume = qBound<int>(0,m_volume + newvol,100);
    else m_volume = newvol;
    emit volumeChanged(m_volume);
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
    if (m_mediastate == PlayState)
        addToCommandQueue("pause 1\n");
    else if (m_mediastate == PauseState)
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

    addToCommandQueue("seek " + QByteArray::number(m_track) + " " + QByteArray::number(m_currenttime) + "\n");
    emit trackChanged(m_trackfile,m_trackname,m_track,m_currenttime,m_totaltime,m_mediastate);
}

qint64 MediaController::getTrackPosition()
{
    return m_currenttime;
}

qint64 MediaController::getTrackDuration()
{
    return m_totaltime;
}

MediaController::MediaState MediaController::state()
{
    return m_mediastate;
}

void MediaController::updatefakepos() {
    m_currenttime+=1;
}

int MediaController::indexOfPlaylist(const QString& name) {
    for (int i=0;i<m_playlists.size();++i) {
        if (m_playlists[i] == name) return i;
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
        m_playlists.append(m_tempplaylists[i]);
        emit playlistsChanged(m_playlists[i], m_playlists.size());
    }

    //remove old
    for (int i=vorhanden.size()-1;i>=0;--i) {
        if (vorhanden[i] == true) continue;
        emit playlistsChanged(m_playlists[i], -1);
        m_playlists.removeAt(i);
    }

    m_tempplaylists.clear();
}
void MediaController::dumpMediaInfo() {
    m_needCurrentSong = StatusNeed;
    m_mpdstatus->write("noidle\n");
}

void MediaController::saveMediaInfo() {
    QDir m_savedir = QDir::home().filePath ( QLatin1String("mediainfo") );
    if ( !m_savedir.exists() && !m_savedir.mkpath ( m_savedir.absolutePath() ) )
    {
        qDebug() << "MPD: saveMediaInfo failed in" << m_savedir;
        return;
    }

    QFile namefile(m_savedir.absoluteFilePath ( QDateTime::currentDateTime().toString() ));
    namefile.open(QIODevice::WriteOnly|QIODevice::Truncate);
    namefile.write("File: "+m_trackfile.toUtf8()+"\nTitle: "+m_tracktitle.toUtf8()+"\nName: "+m_trackname.toUtf8());
    namefile.close();
}

