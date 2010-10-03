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
#include "playlist.h"
#include <QSettings>
#include <limits>
#include "factory.h"
#include "RoomControlServer.h"
#include "stateTracker/mediastatetracker.h"
#include <stateTracker/volumestatetracker.h>
#include <qprocess.h>
#include <stateTracker/pastatetracker.h>

MediaController::MediaController() : m_mediastate(StopState), m_currenttime(0), m_totaltime(0)
{
    m_favourite = 0;
    m_mediaStateTracker = new MediaStateTracker(this);
    m_volumestateTracker = new VolumeStateTracker(MEDIAVOLUME_ID, this);
    m_fakepos.setInterval(1000);
    connect(&m_fakepos, SIGNAL(timeout()),SLOT(updatefakepos()));

    m_playerprocess = new QProcess();
    m_playerprocess->setReadChannel(QProcess::StandardOutput);
    connect (m_playerprocess, SIGNAL(error(QProcess::ProcessError)),SLOT(sloterror(QProcess::ProcessError)));
    connect (m_playerprocess, SIGNAL(finished(int)),SLOT(slotdisconnected(int)));
    connect (m_playerprocess, SIGNAL(started()),SLOT(slotconnected()));
    connect (m_playerprocess, SIGNAL(readyRead()),SLOT(slotreadyRead()));
    connect (m_playerprocess, SIGNAL(readyReadStandardError()),SLOT(readyReadStandardError()));

    QSettings settings;
    settings.beginGroup(QLatin1String("media"));
    m_volume = settings.value(QLatin1String("volume"),1.0).toDouble();

    slotdisconnected(0);
}

MediaController::~MediaController()
{
    m_playerprocess->terminate();
    if (!m_playerprocess->waitForFinished())
        m_playerprocess->kill();
    QSettings settings;
    settings.beginGroup(QLatin1String("media"));
    settings.setValue(QLatin1String("volume"),volume());
}


void MediaController::slotconnected()
{
    setVolume(m_volume);
    if (m_current) {
        QByteArray cache;
        foreach (QString file, m_current->files()) {
            cache += "queue "+file.toUtf8()+"\n";
        }
        m_playerprocess->write(cache);
    }
}

void MediaController::slotdisconnected(int)
{
    m_playerprocess->start(QLatin1String("roommedia"));
}

void MediaController::sloterror(QProcess::ProcessError)
{
    qWarning()<<"MediaPlayer process communication error";
}

void MediaController::readyReadStandardError()
{
    qWarning()<<"MediaPlayer error:" << m_playerprocess->readAllStandardError().replace('\n',' ');
}

void MediaController::slotreadyRead()
{
    bool needsync = false;
    while (!m_playerprocess->atEnd())
    {
        QByteArray line = m_playerprocess->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');

        if (args[0] == "state" && args.size()==2) {
			const EnumMediaState before = m_mediastate;
            if (args[1]=="playing") {
                m_mediastate = PlayState;
                m_fakepos.start();
            }
            else if (args[1]=="paused") {
                m_mediastate = PauseState;
                m_fakepos.stop();
            }
            else if (args[1]=="stopped") {
                m_mediastate = StopState;
                m_fakepos.stop();
            }
            if (before!=m_mediastate) needsync = true;
        } else if (args[0] == "volume" && args.size()==2) {
            m_volume = args[1].toDouble();
            m_volumestateTracker->sync(m_volume);
        } else if (args[0] == "position" && args.size()==2) {
			const int temp = args[1].toInt();
			if (m_totaltime!=temp) needsync = true;
            m_currenttime = temp;
        } else if (args[0] == "total" && args.size()==2) {
			const int temp = args[1].toInt();
            if (m_totaltime!=temp) needsync = true;
            m_totaltime = temp;
        } else if (args[0] == "active" && args.size()==2) {
            m_current->setCurrentTrack(args[1].toInt());
            m_currenttime = 0;
            needsync = true;
        } else if (args[0] == "pa_sink" && args.size()==4) {
            PAStateTracker* p = m_paStateTrackers[args[1]];
            if (!p) {
                p = new PAStateTracker(QString::fromAscii(args[1]), this);
                m_paStateTrackers.insert(args[1], p);
            }
			p->setMute(args[2].toInt());
			p->setVolume(args[3].toUInt()/10000.0);
			p->sync();
        }
    }

    if (needsync) {
		//qDebug() << __FUNCTION__ << "SYNC" << m_currenttime << m_totaltime;
        m_mediaStateTracker->sync();
    }
}

QList<AbstractStateTracker*> MediaController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    l.append(m_mediaStateTracker);
    l.append(m_volumestateTracker);
	QList<PAStateTracker*> k = m_paStateTrackers.values();
    foreach(PAStateTracker* p, k) l.append(p);
    return l;
}

void MediaController::activateFavourite()
{
    if (!m_favourite)
    {
        // create favourite playlist
        m_favourite = new Playlist();
        m_favourite->setName(QLatin1String("favourite"));
        RoomControlServer::getFactory()->addServiceProvider(m_favourite);
        RoomControlServer::getFactory()->objectSaveToDisk(m_favourite);
    }
    setPlaylist(m_favourite);
}

void MediaController::nextPlaylist()
{
    if (!m_current) return;
    int index = m_items.indexOf(m_current)+1;
    if (index>=m_items.size()) index = 0;
    setPlaylistByIndex(index);
}

void MediaController::previousPlaylist()
{
    if (!m_current) return;
    int index = m_items.indexOf(m_current)-1;
    if (index<0) index = m_items.size()-1;
    setPlaylistByIndex(index);
}

void MediaController::setPlaylistByIndex ( int index )
{
    setPlaylist ( m_items.at ( index ) );
}

void MediaController::setPlaylistByID ( const QString& id )
{
    AbstractServiceProvider* p = RoomControlServer::getFactory()->get(id);
    if (!p) return;
    Playlist* pl = qobject_cast<Playlist*>(p);
    if (!pl) return;
    setPlaylist ( pl );
}

void MediaController::setPlaylist ( Playlist* current )
{
    if (!current) return;
    if (current == m_current && !m_current->hasChanged()) return;
    m_current = current;
	m_current->setChanged(false);
    m_mediaStateTracker->sync();
    QByteArray cache = "clear\n";
    if (m_current)
        foreach (QString file, m_current->files()) {
        cache += "queue "+file.toUtf8()+"\n";
    }
    m_playerprocess->write(cache);
}

Playlist* MediaController::playlist()
{
    return m_current;
}

void MediaController::setVolume ( qreal newvol, bool relative )
{
    if (relative) m_volume = qBound<qreal>(0.0,m_volume+= newvol,1.0);
    else m_volume = newvol;
    m_volumestateTracker->sync(m_volume);
    char t[20];
    sprintf((char*)t, "volume %f\n", m_volume);
//	qDebug() << __FUNCTION__<< relative << newvol << m_volume << t;
    m_playerprocess->write(t);
}

qreal MediaController::volume() const
{
    return m_volume;
}

void MediaController::next()
{
    if (!m_current) return;
    const int track = m_current->currentTrack();
    m_current->setCurrentTrack(track+1);
    play();
}

void MediaController::previous()
{
    if (!m_current) return;
    const int track = m_current->currentTrack();
    m_current->setCurrentTrack(track-1);
    play();
}

void MediaController::stop()
{
    m_playerprocess->write("stop\n");
}

void MediaController::pause()
{
    if (m_mediastate == PlayState)
        m_playerprocess->write("pause\n");
    else
        m_playerprocess->write("play\n");
}

void MediaController::play()
{
    if (!m_current || m_current->currentTrack()<0) return;
    setPlaylist(m_current);

    const QByteArray filename = m_current->currentFilename().toUtf8();
    if (filename.isEmpty()) return;
    m_playerprocess->write("select " + QByteArray::number(m_current->currentTrack()) +"\n");
}

void MediaController::setTrackPosition ( qint64 pos, bool relative )
{
	// Don't allow seek if total runtime is not known
	// Instead ask media process again for the total runtime
	if (m_totaltime==0) {
		m_playerprocess->write("getposition\n");
		return;
	}
	//qDebug() << __FUNCTION__ << m_currenttime << m_totaltime;
	
    if (relative) m_currenttime += pos;
    else m_currenttime = pos;
    m_playerprocess->write(QString(QLatin1String("position %1\n")).arg(m_currenttime).toAscii());
    m_mediaStateTracker->sync();
}

qint64 MediaController::getTrackPosition()
{
    return m_currenttime;
}

qint64 MediaController::getTrackDuration()
{
    return m_totaltime;
}

EnumMediaState MediaController::state()
{
    return m_mediastate;
}

void MediaController::addedProvider(AbstractServiceProvider* provider)
{
    Playlist* p = qobject_cast<Playlist*>(provider);
    if (!p) return;
    if (p->name() == QLatin1String("favourite"))
        m_favourite = p;

    m_items.append(p);
}

void MediaController::removedProvider(AbstractServiceProvider* provider)
{
    Playlist* p = qobject_cast<Playlist*>(provider);
    if (!p) return;
    m_items.removeAll(p);
}

int MediaController::count()
{
    return m_items.count();
}

void MediaController::setPAMute(const QByteArray sink, bool mute) {
    m_playerprocess->write("pa_mute " + sink + " " + QByteArray::number(mute) +"\n");
}

void MediaController::togglePAMute(const QByteArray sink) {
    m_playerprocess->write("pa_mute " + sink +" 2\n");
}

void MediaController::setPAVolume(const QByteArray sink, double volume, bool relative) {
    if (relative)
        m_playerprocess->write("pa_volume_relative " + sink + " " + QByteArray::number(volume*100) + "\n");
    else {
        m_playerprocess->write("pa_volume " + sink + " " + QByteArray::number(volume*100) + "\n");
	}
}
void MediaController::updatefakepos() {
	m_currenttime+=1000;
}

