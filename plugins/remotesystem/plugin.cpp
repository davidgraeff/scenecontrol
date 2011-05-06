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
#include "externalclient.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    _config ( this );
}

plugin::~plugin() {
    qDeleteAll ( m_clients );
    m_clients.clear();
}

void plugin::clear() {}
void plugin::initialize() {

}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if ( name == QLatin1String ( "servers" ) ) {
        qDeleteAll ( m_clients );
        m_clients.clear();
        QStringList strings = value.toString().split ( QLatin1Char ( ';' ) );
        foreach ( QString address, strings ) {
            const QStringList data ( address.split ( QLatin1Char ( ':' ) ) );
            if ( data.size() !=2 ) continue;
            ExternalClient* client = new ExternalClient ( this,data[0], data[1].toInt() );
            connect(client, SIGNAL(stateChanged(ExternalClient*)),SLOT(stateChanged(ExternalClient*)));
            m_clients.append ( client );
        }
        // default: select all clients
        m_selectedclients = m_clients;
    }
}

void plugin::execute ( const QVariantMap& data, const QString& sessionid ) {
	Q_UNUSED(sessionid);
    if ( ServiceID::isId(data, "remotefocus" ) ) {
        m_selectedclients = specificClients ( DATA ( "servers" ).split ( QLatin1Char ( ';' ) ) );
    } else if ( ServiceID::isId(data, "remotevolume" ) ) {
        foreach ( ExternalClient* client, m_selectedclients ) {
            client->setSystemVolume ( DOUBLEDATA ( "volume" ),BOOLDATA ( "relative" ) );
        }
    } else if ( ServiceID::isId(data, "remotenotification" ) ) {
        foreach ( ExternalClient* client, m_selectedclients ) {
            client->showMessage ( INTDATA ( "duration" ), DATA ( "title" ), DATA ( "audio" ) );
        }
    } else if ( ServiceID::isId(data, "remotevideo" ) ) {
        foreach ( ExternalClient* client, m_selectedclients ) {
            client->showVideo ( DATA ( "video" ), INTDATA ( "display" ) );
        }
    } else if ( ServiceID::isId(data, "remotedisplay" ) ) {
        foreach ( ExternalClient* client, m_selectedclients ) {
            client->setDisplayState ( INTDATA ( "state" ), INTDATA ( "display" ) );
        }
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
	l.append(ServiceCreation::createModelReset(PLUGIN_ID, "remote.connection.state", "server").getData());
    foreach (ExternalClient* client, m_clients) {
        l.append(stateChanged(client, false));
    }
    return l;
}

QList< ExternalClient* > plugin::specificClients ( const QStringList& hosts ) {
    QList<ExternalClient*> r;
    foreach ( QString host, hosts ) {
        foreach ( ExternalClient* client, m_clients ) {
            if ( client->peerAddress() == QHostAddress ( host ) ) r.append ( client );
        }
    }
    return r;
}

QVariantMap plugin::stateChanged(ExternalClient* client, bool propagate) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "remote.connection.state");
	const QString server = client->host()+QLatin1String(":")+QString::number(client->port());
    sc.setData("server",server);
    sc.setData("state", (int)client->isConnected());
    if (propagate) m_server->property_changed(sc.getData());
    return sc.getData();
}
