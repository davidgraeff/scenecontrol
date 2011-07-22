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

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() : m_events(QLatin1String("pin")) {
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
    if ( name == QLatin1String ( "port" ) ) {
        m_sensors.resize(8);
        delete m_socket;
        m_socket = new QUdpSocket(this);
        connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
        m_socket->bind(QHostAddress::Broadcast,value.toInt());

		char b[] = {'W','A'};
        m_socket->write( b );
        m_socket->flush();
    }
}

void plugin::execute ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED ( data );
    Q_UNUSED ( sessionid );
}

bool plugin::condition ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( data );
    Q_UNUSED ( sessionid );
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(collectionuid);
    if (ServiceID::isId(data,"udpwatchio.value")) {
        m_events.add(data, collectionuid);
    }
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
    m_events.remove(data, collectionuid);
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "udpwatchio.sensor");
        for (int id=0;id<m_sensors.size();++id) {
            sc.setData("sensorid", id);
            sc.setData("value", m_sensors[id]);
            l.append(sc.getData());
        }
    }
    return l;
}

void plugin::readyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size() );
        if (bytes.size() < 3 || bytes[0] != 'w' || bytes[1] != 'a')
            continue;
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "udpwatchio.sensor");
        const char d = bytes[3];
        for (int i=0;i<8;++i) {
			bool newvalue = (1 << i) & d;
			if (m_sensors[i] != newvalue) {
				m_sensors[i] = newvalue;
				sc.setData("sensorid", i);
				sc.setData("value", m_sensors[i]);
				m_server->property_changed(sc.getData());
				m_events.triggerEvent(i, m_server);
			}
        }
    }
}
