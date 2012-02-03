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
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

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
    clear();
}

void plugin::clear() {
    m_clients.clear();
}
void plugin::initialize() {}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
  Q_UNUSED(configid);
    if (!data.contains(QLatin1String("listenport")) || !data.contains(QLatin1String("broadcastport")))
        return;

    clear();
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

void plugin::requestProperties(int sessionid) {
    changeProperty(ServiceData::createModelReset("remote.connection.state", "server").getData(), sessionid);
    QMap<QString, ExternalClient>::const_iterator i = m_clients.constBegin();
    for (;i != m_clients.constEnd(); ++i) {
        changeProperty(stateChanged(&(*i), false), sessionid);
    }
}

inline QVariantMap plugin::stateChanged(const ExternalClient* client, bool propagate) {
    ServiceData sc = ServiceData::createModelChangeItem("remote.connection.state");
    sc.setData("host",client->host.toString());
    sc.setData("port",client->port);
    sc.setData("state", (int)client->noResponse);
    if (propagate) changeProperty(sc.getData());
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
void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    if (plugin_id != COMSERVERSTRING) {
        qWarning() << "Plugin" << pluginid() << "only controllable by the server";
        return;
    }

    QMap<QString, ExternalClient>::const_iterator i = m_clients.constBegin();

    if (!m_allowedmembers.contains(ServiceData::method(data))) {
        qWarning() << "Unallowed memeber action requested" << data;
        return;
    }

    QByteArray d;
    QDataStream stream(&d, QIODevice::WriteOnly);
    stream << data;

    for (;i != m_clients.constEnd(); ++i) {
        const ExternalClient* c = &(*i);
        QUdpSocket().writeDatagram(d, c->host, c->port);
    }
}
