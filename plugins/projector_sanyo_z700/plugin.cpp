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
#include "configplugin.h"
#include <qfile.h>
#include <shared/qextserialport/qextserialport.h>

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_socket = 0;
    _config ( this );
}

plugin::~plugin() {
    delete m_socket;
}

void plugin::clear() {}
void plugin::initialize() {

}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if ( name == QLatin1String ( "server" ) ) {
        const QString server = value.toString();
        const int v = server.indexOf(QLatin1Char(':'));
        if (v==-1) {
            qWarning() << pluginid() << "Configuration wrong (server:port)" << server;
            return;
        }
        delete m_socket;
        m_socket = new QUdpSocket(this);
        connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
        m_socket->bind(QHostAddress(server.mid(0,v)),server.mid(v+1).toInt());
    }
}

void plugin::execute ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED(sessionid);
    if ( !m_socket ) return;
    static char b[] = {'C', 0, 0, '\r'};

    if ( ServiceID::isId(data, "projector_sanyo_power" ) ) {
        if ( BOOLDATA ( "power" ) )
            strncpy ( b, "C00", 3 );
        else
            strncpy ( b, "C01", 3 );
        m_socket->write(b, sizeof(b));
    } else if ( ServiceID::isId(data, "projector_sanyo_video" ) ) {
        if ( BOOLDATA ( "mute" ) )
            strncpy ( b, "C0D", 3 );
        else
            strncpy ( b, "C0E", 3 );
        m_socket->write(b, sizeof(b));
    } else if ( ServiceID::isId(data, "projector_sanyo_lamp" ) ) {
        if ( BOOLDATA ( "eco" ) )
            strncpy ( b, "C75", 3 );
        else
            strncpy ( b, "C74", 3 );
        m_socket->write(b, sizeof(b));
    }
}

bool plugin::condition ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( data );
    Q_UNUSED(sessionid);
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED ( data );
    Q_UNUSED(collectionuid);
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(data);
    Q_UNUSED(collectionuid);
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    return l;
}

void plugin::readyRead() {
    QByteArray bytes;
    int a = m_socket->bytesAvailable();
    bytes.resize ( a );
    m_socket->read ( bytes.data(), bytes.size() );
}
