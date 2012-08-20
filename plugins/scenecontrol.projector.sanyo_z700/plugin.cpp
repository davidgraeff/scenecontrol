/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QDebug>

#include "plugin.h"
#include <qfile.h>
#include "shared/serialdevice/qxtserialdevice.h"

#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid) {
    m_serial = 0;
    m_buffer[3] = '\r';
}

plugin::~plugin() {
    delete m_serial;
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data)
{
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("serialport"))) {
        delete m_serial;
		m_serial = 0;
        const QString device = data[QLatin1String("serialport")].toString();
        if ( !QFile::exists ( device ) ) {
            qWarning() << pluginid() << "device not found" << device;
            return;
        }
        m_serial = new QxtSerialDevice ( device );
        m_serial->setBaud ( QxtSerialDevice::Baud19200 );
        m_serial->setPortSettings ( QxtSerialDevice::FlowOff | QxtSerialDevice::ParityNone
                                    | QxtSerialDevice::Stop1 | QxtSerialDevice::Bit8);

        if ( !m_serial->open ( QIODevice::ReadWrite ) ) {
            qWarning() << pluginid() << "rs232 error:" << m_serial->errorString();
        } else {
			connect ( m_serial, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
			qDebug() << pluginid() << "connected to"<<device;
        }
    }
}

void plugin::writeToDevice() {
    if (!m_serial)
        return;
    if ( !m_serial->write ( m_buffer,4 ) ) {
        qWarning() << pluginid() << "send failed\n";
        return;
    }
}

void plugin::readyRead() {
    QByteArray bytes;
    int a = m_serial->bytesAvailable();
    bytes.resize ( a );
    m_serial->read ( bytes.data(), bytes.size() );
}

void plugin::projector_sanyo_power(bool power) {
    if ( !m_serial ) return;
    if (power)
        strncpy ( m_buffer, "C00", 3 );
    else
        strncpy ( m_buffer, "C01", 3 );
    writeToDevice();
}

void plugin::projector_sanyo_video(bool mute) {
    if ( !m_serial ) return;
    if (mute)
        strncpy ( m_buffer, "C0D", 3 );
    else
        strncpy ( m_buffer, "C0E", 3 );
    writeToDevice();
}

void plugin::projector_sanyo_lamp(bool eco) {
    if ( !m_serial ) return;
    if (eco)
        strncpy ( m_buffer, "C75", 3 );
    else
        strncpy ( m_buffer, "C74", 3 );
    writeToDevice();
}
