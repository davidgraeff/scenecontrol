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

#ifndef EventStateTracker_h
#define EventStateTracker_h
#include <shared/abstractstatetracker.h>

class EventStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename WRITE setFilename);
    Q_PROPERTY(QString title READ title WRITE setTitle);
    Q_PROPERTY(int state READ state WRITE setState);
public:
    EventStateTracker(QObject* parent = 0);
    QString title() const {
        return m_title;
    }
    void setTitle(const QString& t) {
        m_title = t;
    }
    QString filename() const {
        return m_filename;
    }
    void setFilename(const QString& f) {
        m_filename = f;
    }
    int state() const {
        return m_state;
    }
    void setState(int vol) {
        m_state = vol;
    }
private:
    int m_state;
    QString m_title;
    QString m_filename;
};

#endif // EventStateTracker_h
