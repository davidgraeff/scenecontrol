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

#ifndef PROJECTORCONTROLLER_H
#define PROJECTORCONTROLLER_H

#include <QObject>
#include "qextserialport/qextserialport.h"

class QextSerialPort;
class AbstractStateTracker;
class ProjectorStateTracker;
enum ProjectorControl {
    ProjectorOn,
    ProjectorOff,
    ProjectorVideoMute,
    ProjectorVideoUnMute,
    ProjectorLampNormal,
    ProjectorLampEco
};

class ProjectorController : public QObject
{
	Q_OBJECT
public:
    ProjectorController();
    ~ProjectorController();
    void setCommand(ProjectorControl c);
    QList<AbstractStateTracker*> getStateTracker();
	QextSerialPort m_serial;
private:
	char buffer[4];
	ProjectorStateTracker* stateTracker;
public slots:
    void readyRead();
};

#endif // PROJECTORCONTROLLER_H
