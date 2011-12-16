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
#include <QtPlugin>

#include "plugin.h"
#include <qfile.h>
#include <shared/qextserialport/qextserialport.h>

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_serial = 0;
    m_buffer[3] = '\r';
}

plugin::~plugin() {
    delete m_serial;
}

void plugin::clear() {}
void plugin::initialize() {

}

void plugin::settingsChanged(const QVariantMap& data)
{
    if (data.contains(QLatin1String("serialport"))) {
        delete m_serial;
        const QString device = data[QLatin1String("serialport")].toString();
        if ( !QFile::exists ( device ) ) {
            qWarning() << pluginid() << "device not found" << device;
            return;
        }
        m_serial = new QextSerialPort ( device,QextSerialPort::EventDriven );
        m_serial->setBaudRate ( BAUD19200 );
        m_serial->setFlowControl ( FLOW_OFF );
        m_serial->setParity ( PAR_NONE );
        m_serial->setDataBits ( DATA_8 );
        m_serial->setStopBits ( STOP_1 );
        connect ( m_serial, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
        if ( !m_serial->open ( QIODevice::ReadWrite ) ) {
            qWarning() << pluginid() << "rs232 error:" << m_serial->errorString();
        } else {
            qDebug() << "sanyo connected to"<<device;
        }
    }
}

void plugin::writeToDevice() {
    if ( !m_serial->write ( m_buffer,4 ) ) {
        qWarning() << pluginid() << "send failed\n";
        return;
    }
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED(sessionid);
    if ( !m_serial ) return;
    if ( ServiceID::isMethod(data, "projector_sanyo_power" ) ) {
        if ( BOOLDATA ( "power" ) )
            strncpy ( m_buffer, "C00", 3 );
        else
            strncpy ( m_buffer, "C01", 3 );
        writeToDevice();
    } else if ( ServiceID::isMethod(data, "projector_sanyo_video" ) ) {
        if ( BOOLDATA ( "mute" ) )
            strncpy ( m_buffer, "C0D", 3 );
        else
            strncpy ( m_buffer, "C0E", 3 );
        writeToDevice();
    } else if ( ServiceID::isMethod(data, "projector_sanyo_lamp" ) ) {
        if ( BOOLDATA ( "eco" ) )
            strncpy ( m_buffer, "C75", 3 );
        else
            strncpy ( m_buffer, "C74", 3 );
        writeToDevice();
    } else if ( ServiceID::isMethod(data, "projector_sanyo_focus" ) ) {
        //TODO
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( data );
    Q_UNUSED(sessionid);
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED ( data );
    Q_UNUSED(collectionuid);
    Q_UNUSED(sessionid);
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(eventid);
    Q_UNUSED(sessionid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    return l;
}

void plugin::readyRead() {
    QByteArray bytes;
    int a = m_serial->bytesAvailable();
    bytes.resize ( a );
    m_serial->read ( bytes.data(), bytes.size() );
}
