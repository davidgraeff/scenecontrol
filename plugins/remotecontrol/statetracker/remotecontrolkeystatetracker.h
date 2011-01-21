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

#ifndef REMOTECONTROLKEYSTATETRACKER_H
#define REMOTECONTROLKEYSTATETRACKER_H
#include <shared/abstractstatetracker.h>

class RemoteControlKeyStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey);
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed);
    Q_PROPERTY(int channel READ channel WRITE setChannel);
public:
    RemoteControlKeyStateTracker(QObject* parent = 0);
    void setKey(const QString& key) {
        m_key = key;
    }
    QString key() const {
        return m_key;
    }
    bool pressed() const {
        return m_pressed;
    }
    int channel() const {
        return m_channel;
    }
    void setPressed(bool b) {
        m_pressed = b;
    }
    void setChannel(int r) {
        m_channel = r;
    }
private:
    QString m_key;
    bool m_pressed;
    int m_channel;
};

#endif // REMOTECONTROLKEYSTATETRACKER_H
