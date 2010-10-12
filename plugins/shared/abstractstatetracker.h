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

#ifndef ABSTRACTSTATETRACKER_H
#define ABSTRACTSTATETRACKER_H
#include <QObject>
#include <QStringList>

class AbstractStateTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type);
	Q_PROPERTY(QString plugin READ plugin WRITE setPlugin);
public:
    AbstractStateTracker(QObject* parent = 0);
    /**
      * Free this state tracker and share this info with clients
      */
    virtual ~AbstractStateTracker();

    QString type() const;
	QString plugin() const { return m_plugin; }
	void setPlugin(const QString& plugin) { m_plugin = plugin; }
private:
	QString m_plugin;
};

#endif // ABSTRACTSTATETRACKER_H
