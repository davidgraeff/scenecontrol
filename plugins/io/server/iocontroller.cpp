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
#include "plugin_server.h"
#include <shared/abstractplugin.h>

#define DEVICE "/dev/ttyUSB0"
IOController::IOController(myPluginExecute* plugin) : m_pluginname(plugin->base()->name()), m_pins(0), m_bufferpos(0), m_readState(ReadOK) {
    QSettings settings;
    settings.beginGroup(m_pluginname);
    m_serial = new QextSerialPort(settings.value(QLatin1String("device"),QLatin1String(DEVICE)).toString(),QextSerialPort::EventDriven);
    m_serial->setBaudRate(BAUD115200);
    m_serial->setFlowControl(FLOW_OFF);
    m_serial->setParity(PAR_NONE);
    m_serial->setDataBits(DATA_8);
    m_serial->setStopBits(STOP_1);
    connect(m_serial, SIGNAL(readyRead()), SLOT(readyRead()));
    // Open device and ask for pins
    if (!QFile::exists(QLatin1String(DEVICE))) {
        qWarning() << "IO: device not found"<<DEVICE;
        return;
    }
    const char t1[] = {0xef};
    if (!m_serial->open(QIODevice::ReadWrite) || !m_serial->write(t1, sizeof(t1))) {
        qWarning() << "IO: rs232 init fehler";
    }
    // panic counter
    m_panicTimer.setInterval(40000);
    connect(&m_panicTimer,SIGNAL(timeout()),SLOT(panicTimeout()));
	m_panicTimer.start();
}

IOController::~IOController() {
    delete m_serial;
    qDeleteAll(m_values);
    qDeleteAll(m_names);
}

void IOController::readyRead() {
    QByteArray bytes;
    bytes.resize(m_serial->bytesAvailable());
    m_serial->read(bytes.data(), bytes.size());
    m_buffer.append(bytes);
    if (m_readState==ReadOK) {
        for (int i=m_bufferpos;i<m_buffer.size();++i) {
            if (m_buffer.size()<=i) break;
            if (m_buffer[i] == 'O' && m_buffer[i+1] == 'K') {
                m_readState = ReadEnd;
                m_buffer.remove(0,i+2);
                m_bufferpos = 0;
                break;
            }
        }
    }
    if (m_readState == ReadEnd) {
        for (int i=m_bufferpos;i<m_buffer.size();++i) {
            if (m_buffer.size()<=i+1) break;
            if (m_buffer[i] == 'E' && m_buffer[i+1] == 'N' && m_buffer[i+2] == 'D') {
                m_readState = ReadOK;
                determinePins(m_buffer.mid(0,i));
                m_buffer.remove(0,i+3);
                m_bufferpos = 0;
                break;
            }
        }
    }
}

void IOController::determinePins(const QByteArray& data) {
	if (data.isEmpty() || data.size() != (int)data[0]+1) {
		qWarning()<<m_pluginname<<__FUNCTION__<<"size missmatch:"<<(data.size()?((int)data[0]+1):0)<<data.size();
        return;
    }
    QSettings settings;
    settings.beginGroup(m_pluginname);
    settings.beginGroup ( QLatin1String("pinnames") );
    // clear old
    qDeleteAll(m_values);
    qDeleteAll(m_names);
    m_values.clear();
    m_names.clear();
    // set new
	m_pins = (int)data[0];
    for ( int i=0;i<m_pins;++i )
    {
        const QString name = settings.value ( QLatin1String("pin")+QString::number( i ),
                                              tr("IO %1").arg( i ) ).toString();
        PinValueStateTracker* cv = new PinValueStateTracker();
        m_values.append(cv);
        cv->setPin(i);
		cv->setValue(data[i+1]);
        emit stateChanged(cv);
        PinNameStateTracker* cn = new PinNameStateTracker();
        m_names.append(cn);
        cn->setPin(i);
        cn->setValue(name);
        emit stateChanged(cn);
    }
    emit dataLoadingComplete();
}

bool IOController::getPin(unsigned int pin) const
{
	if (( unsigned int )m_values.size()<=pin) return false;
    return m_values.at(pin)->value();
}

void IOController::setPin ( uint pin, bool value )
{
	if (( unsigned int )m_values.size()<=pin) return;
    m_values[pin]->setValue(value);
    emit stateChanged(m_values[pin]);
    char a[] = {(value?0xf0:0x00) | (unsigned char)pin};
    m_serial->write(a, 1);
}

void IOController::setPinName ( uint pin, const QString& name )
{
    if ( pin>= ( unsigned int ) m_names.size() ) return;
    m_names[pin]->setValue(name);
    emit stateChanged(m_names[pin]);
    QSettings settings;
	settings.beginGroup(m_pluginname);
    settings.beginGroup ( QLatin1String("pinnames") );
    settings.setValue ( QLatin1String("pin")+QString::number ( pin ), name );
}

void IOController::togglePin ( uint pin )
{
	if (( unsigned int )m_values.size()<=pin) return;
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

QString IOController::getPinName(uint pin) {
    if (pin>=(uint)m_names.size()) return QString();
    return m_names[pin]->value();
}

void IOController::panicTimeout() {
    const char t1[] = {0x00};
    if (!m_serial->isOpen() || m_serial->write(t1, sizeof(t1)) == -1) {
        qWarning()<< "IO: Failed to reset panic counter. Try reconnection";
        m_serial->close();
		const char t1[] = {0xef};
		if (!m_serial->open(QIODevice::ReadWrite) || !m_serial->write(t1,  sizeof(t1))) {
            qWarning() << "IO: rs232 init fehler";
        }
    }
}
