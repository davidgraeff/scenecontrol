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

#ifndef EVENTCONTROLLER_H
#define EVENTCONTROLLER_H

#include <QObject>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>
#include "mediacmds.h"

class VolumeStateTracker;
class EventStateTracker;
class AbstractStateTracker;
using namespace Phonon;

class EventController : public QObject
{
  Q_OBJECT
  public:
    EventController();
    ~EventController();
    bool play();
    EnumMediaState state();

    void setVolume(qreal newvol, bool relative=false);
    qreal volume() const;
    void setTitle(const QString& v);
    QString title() const;
    void setFilename(const QString& v);
    QString filename() const;    

    void sync();
    QList<AbstractStateTracker*> getStateTracker();
  private:
    AudioOutput *m_eventOutput;
    MediaObject *m_eventMedia;
    QString m_eventTitle;
    QString m_filename;
    EventStateTracker* m_stateTracker;
    VolumeStateTracker* m_volumestateTracker;
    Phonon::Path m_audioPath;
    
  private Q_SLOTS:
    void stateChanged ( Phonon::State newstate, Phonon::State oldstate );
    
  public Q_SLOTS:
    void stop();
    
  Q_SIGNALS:
    void started(const QString& eventTitle, const QString& filename);
    void finished(const QString& eventTitle, const QString& filename);
};

#endif // EVENTCONTROLLER_H
