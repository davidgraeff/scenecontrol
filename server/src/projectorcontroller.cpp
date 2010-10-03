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

#include "projectorcontroller.h"
#include <QDebug>
#include "stateTracker/projectorstatetracker.h"

ProjectorController::ProjectorController() :
        m_serial(QLatin1String("/dev/ttyS0"),QextSerialPort::EventDriven) {
    m_serial.setBaudRate(BAUD19200);
    m_serial.setFlowControl(FLOW_OFF);
    m_serial.setParity(PAR_NONE);
    m_serial.setDataBits(DATA_8);
    m_serial.setStopBits(STOP_1);
    connect(&m_serial, SIGNAL(readyRead()), SLOT(readyRead()));
    buffer[3] = '\r';
    if (!m_serial.open(QIODevice::ReadWrite)) {
        qWarning() << "projector rs232 init fehler";
    }
    stateTracker = new ProjectorStateTracker();
}

ProjectorController::~ProjectorController() {
    delete stateTracker;
}

void ProjectorController::setCommand(ProjectorControl c) {
    switch (c) {
    case ProjectorOn:
        strncpy(buffer, "C00", 3);
        break;
    case ProjectorOff:
        strncpy(buffer, "C01", 3);
        break;
    case ProjectorVideoMute:
        strncpy(buffer, "C0D", 3);
        break;
    case ProjectorVideoUnMute:
        strncpy(buffer, "C0E", 3);
        break;
    case ProjectorLampNormal:
        strncpy(buffer, "C74", 3);
        break;
    case ProjectorLampEco:
        strncpy(buffer, "C75", 3);
        break;
    default:
        return;
    };

    if (!m_serial.write(buffer,4)) {
        qWarning() << "projector send failed\n";
        return;
    }

    stateTracker->setState(c);
    stateTracker->sync();
}

QList< AbstractStateTracker* > ProjectorController::getStateTracker() {
    QList<AbstractStateTracker*> temp;
    temp.append(stateTracker);
    return temp;
}

void ProjectorController::readyRead() {
    QByteArray bytes;
    int a = m_serial.bytesAvailable();
    bytes.resize(a);
    m_serial.read(bytes.data(), bytes.size());
}
