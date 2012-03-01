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
#include <shared/json.h>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() {
    m_allowedmembers << QLatin1String("volume.relative"); 	//volume: int
    m_allowedmembers << QLatin1String("volume.absolute"); 	//volume: int
    m_allowedmembers << QLatin1String("display"); 			//power(enum): off, on, toggle
    m_allowedmembers << QLatin1String("mute");    			//mute(enum): off, on, toggle
    m_allowedmembers << QLatin1String("standby");
    m_allowedmembers << QLatin1String("media.start");
	m_allowedmembers << QLatin1String("media.playpause");
    m_allowedmembers << QLatin1String("media.stop");
    m_allowedmembers << QLatin1String("media.next");
    m_allowedmembers << QLatin1String("media.previous");
    m_allowedmembers << QLatin1String("media.playlist.next");
    m_allowedmembers << QLatin1String("media.playlist.previous");
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
    Q_UNUSED(data);
}

void plugin::requestProperties(int sessionid) {
    changeProperty(ServiceData::createModelReset("remote.connection.state", "host").getData(), sessionid);
    QMap<int, ExternalClient>::const_iterator i = m_clients.constBegin();
    for (;i != m_clients.constEnd(); ++i) {
        changeProperty(stateChanged(&(*i), false), sessionid);
    }
}

inline QVariantMap plugin::stateChanged(const ExternalClient* client, bool propagate) {
    ServiceData sc = ServiceData::createModelChangeItem("remote.connection.state");
    sc.setData("host",client->host);
    sc.setData("identifier",client->identifier);
    if (propagate) changeProperty(sc.getData());
    return sc.getData();
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    if (plugin_id != COMSERVERSTRING) {
        qWarning() << "Plugin" << pluginid() << "only controllable by the server";
        return;
    }

    QMap<int, ExternalClient>::const_iterator i = m_clients.constBegin();

    if (!m_allowedmembers.contains(ServiceData::method(data))) {
        qWarning() << "Unallowed memeber action requested" << data;
        return;
    }

    for (;i != m_clients.constEnd(); ++i) {
        const ExternalClient* c = &(*i);
        changeProperty(data, c->sessionid);
    }
}

void plugin::registerclient(const QString& host, const QString& identifier) {
    qDebug() << "Session registered" << host << identifier << m_lastsessionid;
    if (m_lastsessionid == -1) {
        return;
    }
    ExternalClient& c = m_clients[m_lastsessionid];
    c.host = host;
    c.identifier = identifier;
    c.sessionid = m_lastsessionid;
    stateChanged(&c, true);
}

void plugin::session_change(int sessionid, bool running) {
    QMap<int, ExternalClient>::const_iterator i = m_clients.find(sessionid);
    if (i == m_clients.end())
        return;

    // Session finished, remove from m_clients
    if (!running) {
        ServiceData sc = ServiceData::createModelRemoveItem("remote.connection.state");
        sc.setData("host",i->host);
        sc.setData("identifier",i->identifier);
	qDebug() << "Session finished" << i->sessionid;
        changeProperty(sc.getData());
        m_clients.remove(sessionid);
    }
}
