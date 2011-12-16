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
#include <qjson/serializer.h>

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    connect(&m_listenSocket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(&m_checkClientTimer, SIGNAL(timeout()), SLOT(checkClientAlive()));
    m_checkClientTimer.setSingleShot(true);
    m_checkClientTimer.setInterval(60000);

    m_allowedmembers << QLatin1String("volume_relative");
    m_allowedmembers << QLatin1String("volume_absolute");
    m_allowedmembers << QLatin1String("display_off");
    m_allowedmembers << QLatin1String("display_on");
    m_allowedmembers << QLatin1String("mute");
    m_allowedmembers << QLatin1String("unmute");
    m_allowedmembers << QLatin1String("togglemute");
    m_allowedmembers << QLatin1String("standby");
    m_allowedmembers << QLatin1String("startmedia");
    m_allowedmembers << QLatin1String("stopmedia");
    m_allowedmembers << QLatin1String("nextmedia");
    m_allowedmembers << QLatin1String("previousmedia");
    m_allowedmembers << QLatin1String("nextplaylistmedia");
    m_allowedmembers << QLatin1String("previousplaylistmedia");

}

plugin::~plugin() {
    m_clients.clear();
}

void plugin::clear() {}
void plugin::initialize() {}

void plugin::settingsChanged(const QVariantMap& data) {
    if (!data.contains(QLatin1String("listenport")) || !data.contains(QLatin1String("broadcastport")))
        return;

    m_listenSocket.bind(data[QLatin1String("listenport")].toInt());
    m_listenSocket.writeDatagram("ROOMCONTROLCLIENTS\n", QHostAddress::Broadcast, data[QLatin1String("broadcastport")].toInt());
}

void plugin::readyRead() {
    while (m_listenSocket.hasPendingDatagrams()) {
        QByteArray bytes;
        QHostAddress host;
        quint16 port;
        bytes.resize ( m_listenSocket.pendingDatagramSize() );
        m_listenSocket.readDatagram ( bytes.data(), bytes.size(), &host, &port );

        if (bytes == "IAMAROOMCONTROLCLIENT\n") {
            ExternalClient* c = &m_clients[host.toString()];
            const bool changed = (c->host != host);
            c->host = host;
            c->port = port;
            c->noResponse = false;
            m_checkClientTimer.start();
            if (changed)
                stateChanged(c, true);
	}
    }
}


void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED(sessionid);
    QMap<QString, ExternalClient>::const_iterator i = m_clients.constBegin();

    if (!m_allowedmembers.contains(ServiceID::pluginmember(data))) {
        qWarning() << "Unallowed memeber action requested" << data;
        return;
    }

    const QByteArray d = QJson::Serializer().serialize(data);

    for (;i != m_clients.constEnd(); ++i) {
        const ExternalClient* c = &(*i);
        QUdpSocket().writeDatagram(d, c->host, c->port);
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( data );
    Q_UNUSED(sessionid);
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED ( data );
    Q_UNUSED(collectionuid);
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(eventid);
    Q_UNUSED(sessionid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "remote.connection.state", "server").getData());
    QMap<QString, ExternalClient>::const_iterator i = m_clients.constBegin();
    for (;i != m_clients.constEnd(); ++i) {
        l.append(stateChanged(&(*i), false));
    }
    return l;
}

inline QVariantMap plugin::stateChanged(const ExternalClient* client, bool propagate) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "remote.connection.state");
    sc.setData("host",client->host.toString());
    sc.setData("port",client->port);
    sc.setData("state", (int)client->noResponse);
    if (propagate) m_serverPropertyController->pluginPropertyChanged(sc.getData());
    return sc.getData();
}

void plugin::checkClientAlive() {
    QMutableMapIterator<QString, ExternalClient> i(m_clients);
    while (i.hasNext())  {
        i.next();
        ExternalClient* c = &(i.value());
        if (c->noResponse) {
            i.remove();
            continue;
        }
        c->noResponse = true;
        QUdpSocket().writeDatagram("ROOMCONTROLCLIENTS\n", c->host, c->port);
    }

    if (m_clients.size())
        m_checkClientTimer.start();
}
