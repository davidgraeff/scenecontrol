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

#ifndef PROGRAMSTATETRACKER_H
#define PROGRAMSTATETRACKER_H
#include "shared/abstractstatetracker.h"

class SystemStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString appversion READ appversion WRITE setApp);
    Q_PROPERTY(QString minversion READ minversion WRITE setMin);
    Q_PROPERTY(QString maxversion READ maxversion WRITE setMax);
public:
    SystemStateTracker(QObject* parent = 0);
    QString appversion() const;
    QString minversion() const;
    QString maxversion() const;
	void setApp(const QString& a);
	void setMin(const QString& a);
	void setMax(const QString& a);
private:
	QString m_app;
	QString m_min;
	QString m_max;
};

#endif // PROGRAMSTATETRACKER_H
