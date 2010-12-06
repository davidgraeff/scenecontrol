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

#include "iocontroller.h"
#include <QDebug>
#include <QSettings>
#include "shared/server/qextserialport.h"
#include "statetracker/pinvaluestatetracker.h"
#include "statetracker/pinnamestatetracker.h"
#include <qfile.h>

#define DEVICE "/dev/ttyUSB0"
IOController::IOController() :
        m_serial(QLatin1String(DEVICE),QextSerialPort::EventDriven) {
    m_serial.setBaudRate(BAUD115200);
    m_serial.setFlowControl(FLOW_OFF);
    m_serial.setParity(PAR_NONE);
    m_serial.setDataBits(DATA_8);
    m_serial.setStopBits(STOP_1);
    connect(&m_serial, SIGNAL(readyRead()), SLOT(readyRead()));
    m_pins = 0;
    // Open device and ask for pins
    const char t1[] = {0xef};
    if (!QFile::exists(QLatin1String(DEVICE))) {
      qWarning() << "IO: device not found"<<DEVICE;
	return;
    }
    if (!m_serial.open(QIODevice::ReadWrite) || !m_serial.write(t1, sizeof(t1))) {
        qWarning() << "IO: rs232 init fehler";
    }
}

IOController::~IOController() {
    qDeleteAll(m_values);
    qDeleteAll(m_names);
}

void IOController::readyRead() {
    QByteArray bytes;
    bytes.resize(m_serial.bytesAvailable());
    m_serial.read(bytes.data(), bytes.size());
    m_buffer.append(bytes);
    //qDebug() << "Read:" << m_buffer.size() << m_buffer;
    if (m_buffer.size()>=3) {
        for (int i=m_buffer.size()-1;i>1;--i) {
            if (m_buffer[i-2]=='O' && m_buffer[i-1]=='K') {
                m_pins = (int)m_buffer[i];
                m_buffer.truncate(i-1);
                getPins();
            }
        }
        m_buffer.resize(4);
    }
}

void IOController::getPins() {
    QSettings settings;
    qDeleteAll(m_values);
    qDeleteAll(m_names);
    m_values.clear();
    m_names.clear();
    settings.beginGroup ( QLatin1String("pinnames") );
    for ( int i=0;i<m_pins;++i )
    {
        const QString name = settings.value ( QLatin1String("pin")+QString::number( i ),
                                              tr("IO %1").arg( i ) ).toString();
        PinValueStateTracker* cv = new PinValueStateTracker();
        m_values.append(cv);
        cv->setPin(i);
        cv->setValue(0);
        emit stateChanged(cv);
        PinNameStateTracker* cn = new PinNameStateTracker();
        m_names.append(cn);
        cn->setPin(i);
        cn->setValue(name);
        emit stateChanged(cn);
    }
}


bool IOController::getPin(unsigned int pin) const
{
    return m_values.at(pin)->value();
}

void IOController::setPin ( uint pin, bool value )
{
    m_values[pin]->setValue(value);
    emit stateChanged(m_values[pin]);
    char a[] = {(value?0xf0:0x00) | (unsigned char)pin};
    m_serial.write(a, 1);
}

void IOController::setPinName ( uint pin, const QString& name )
{
    if ( pin>= ( unsigned int ) m_names.size() ) return;
    m_names[pin]->setValue(name);
    emit stateChanged(m_names[pin]);
    QSettings settings;
    settings.beginGroup ( QLatin1String("pinnames") );
    settings.setValue ( QLatin1String("pin")+QString::number ( pin ), name );
}

void IOController::togglePin ( uint pin )
{
    setPin ( pin, !m_values[pin]->value() );
}

QList< AbstractStateTracker* > IOController::getStateTracker() {
    QList<AbstractStateTracker*> temp;
    foreach (PinValueStateTracker* p, m_values) temp.append(p);
    foreach (PinNameStateTracker* p, m_names) temp.append(p);
    return temp;
}

int IOController::countPins() {
    return m_values.size();
}
void IOController::setLedState(bool state) {
    const char t1[] = {(state)?0xcf:0xdf};
    m_serial.write(t1, sizeof(t1));
}

QString IOController::getPinName(uint pin) {
	if (pin>=(uint)m_names.size()) return QString();
    return m_names[pin]->value();
}
