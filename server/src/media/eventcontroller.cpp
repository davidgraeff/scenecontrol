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

#include "eventcontroller.h"
#include "stateTracker/abstractstatetracker.h"
#include <stateTracker/eventstatetracker.h>
#include <QSettings>
#include <qfileinfo.h>
#include <stateTracker/volumestatetracker.h>

EventController::EventController()
{
    m_eventOutput = new Phonon::AudioOutput ( Phonon::MusicCategory, this );
    if ( !m_eventOutput->isValid() )
    {
        qWarning() << __FILE__ << "Audio Output not valid";
        return;
    }
    m_eventMedia = new Phonon::MediaObject ( this );
    if ( !m_eventMedia->isValid() )
    {
        qWarning() << __FILE__ << "Audio EventMedia not valid";
        return;
    }
    m_audioPath = Phonon::createPath ( m_eventMedia, m_eventOutput );
    connect(m_eventMedia,SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            SLOT(stateChanged(Phonon::State,Phonon::State)));

    QSettings settings;
    settings.beginGroup(QLatin1String("events"));
    m_eventOutput->setVolume(settings.value(QLatin1String("volume"),1.0).toDouble());

    m_stateTracker = new EventStateTracker(this);
	m_volumestateTracker = new VolumeStateTracker(EVENTVOLUME_ID, this);
}

EventController::~EventController()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("events"));
    settings.setValue(QLatin1String("volume"),m_eventOutput->volume());
    delete m_eventMedia;
    delete m_eventOutput;
}

QList<AbstractStateTracker*> EventController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    l.append(m_stateTracker);
	l.append(m_volumestateTracker);
    return l;
}

void EventController::setVolume ( qreal newvol, bool relative )
{
    if (!relative)
        m_eventOutput->setVolume ( newvol );
    else
        m_eventOutput->setVolume ( newvol + volume());
	m_volumestateTracker->sync(volume()*100);
}

qreal EventController::volume() const
{
    const qreal t = m_eventOutput->volume();
    if (t!=t)
        return 0;
    else
        return t;
}

void EventController::setTitle(const QString& v) {
    m_eventTitle = v;
}

QString EventController::title() const {
    return m_eventTitle;
}

void EventController::setFilename(const QString& v) {
    m_filename = v;
}

QString EventController::filename() const
{
    return m_filename;
}

void EventController::sync()
{
    m_stateTracker->sync();
}

void EventController::stop()
{
    m_eventMedia->stop();
}

EnumMediaState EventController::state()
{
    switch (m_eventMedia->state())
    {
    case Phonon::PlayingState:
        return PlayState;
    case Phonon::PausedState:
        return PauseState;
    case Phonon::StoppedState:
        return StopState;
    default:
        return StopState;
    }
}

bool EventController::play ()
{
    if ( m_eventMedia->state() != Phonon::StoppedState ) m_eventMedia->stop();
    if (m_filename.isEmpty() || !QFileInfo(m_filename).exists()) return false;

    m_eventMedia->setCurrentSource ( m_filename );
    if ( m_eventMedia->isValid() )
    {
        m_eventMedia->play();
        return true;
    }
    else
    {
        qWarning() << __FILE__ << "Media Object:" << m_eventMedia->errorString();
        return false;
    }
}

void EventController::stateChanged ( Phonon::State newstate, Phonon::State oldstate )
{
    if (newstate == Phonon::ErrorState) {
	Phonon::MediaObject *oldMediaObject = m_eventMedia;
	m_eventMedia = new Phonon::MediaObject(this);
	m_audioPath.reconnect(m_eventMedia,m_eventOutput);
	connect(m_eventMedia,SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            SLOT(stateChanged(Phonon::State,Phonon::State)));
	delete oldMediaObject;
	qWarning() << "Phonon error; Event sound restarted";
        return;
    }
    if ( oldstate == Phonon::PlayingState && newstate == Phonon::StoppedState )
    {
        emit finished ( m_eventTitle, m_filename );
    }
    else if ( newstate == Phonon::PlayingState && oldstate == Phonon::StoppedState )
    {
        emit started ( m_eventTitle, m_filename );
    }
    m_stateTracker->sync();
}
