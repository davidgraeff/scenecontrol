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

int main ( int argc, char* argv[] )
{
    QCoreApplication app ( argc, argv );
    if ( argc<2 )
    {
        qWarning() <<"No instanceid provided!";
        return 1;
    }
    plugin p ( QLatin1String ( PLUGIN_ID ), QString::fromAscii ( argv[1] ) );
    if ( !p.createCommunicationSockets() )
        return -1;
    return app.exec();
}

plugin::plugin ( const QString& pluginid, const QString& instanceid ) : AbstractPlugin ( pluginid, instanceid )
{
}

plugin::~plugin()
{
    clear();
}

void plugin::clear()
{
    m_clients.clear();
}
void plugin::initialize()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}

void plugin::configChanged ( const QByteArray& configid, const QVariantMap& data )
{
    Q_UNUSED ( configid );
    Q_UNUSED ( data );
}

void plugin::requestProperties ( int sessionid )
{
    changeProperty ( SceneDocument::createModelReset ( "remote.connection.state", "host" ).getData(), sessionid );
    QMap<int, ExternalClient>::const_iterator i = m_clients.constBegin();
    for ( ; i != m_clients.constEnd(); ++i )
    {
        changeProperty ( stateChanged ( & ( *i ), false ), sessionid );
    }
}

inline QVariantMap plugin::stateChanged ( const ExternalClient* client, bool propagate )
{
    SceneDocument sc = SceneDocument::createModelChangeItem ( "remote.connection.state" );
    sc.setData ( "host",client->host );
    sc.setData ( "identifier",client->identifier );
    if ( propagate ) changeProperty ( sc.getData() );
    return sc.getData();
}

void plugin::registerclient ( const QString& host, const QString& identifier )
{
    qDebug() << "Session registered" << host << identifier << m_lastsessionid;
    if ( m_lastsessionid == -1 )
    {
        return;
    }
    ExternalClient& c = m_clients[m_lastsessionid];
    c.host = host;
    c.identifier = identifier;
    c.sessionid = m_lastsessionid;
    stateChanged ( &c, true );
}

void plugin::session_change ( int sessionid, bool running )
{
    QMap<int, ExternalClient>::const_iterator i = m_clients.find ( sessionid );
    if ( i == m_clients.end() )
        return;

    // Session finished, remove from m_clients
    if ( !running )
    {
        SceneDocument sc = SceneDocument::createModelRemoveItem ( "remote.connection.state" );
        sc.setData ( "host",i->host );
        sc.setData ( "identifier",i->identifier );
        qDebug() << "Session finished" << i->sessionid;
        changeProperty ( sc.getData() );
        m_clients.remove ( sessionid );
    }
}

void plugin::sendToClients ( const SceneDocument& sc ) {
    QMap<int, ExternalClient>::const_iterator i = m_clients.constBegin();

    for (;i != m_clients.constEnd(); ++i) {
        const ExternalClient* c = &(*i);
        changeProperty(sc.getData(), c->sessionid);
    }
}

void plugin::mute ( int mute )
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
	sc.setData("mute", mute);
    sendToClients ( sc );
}
void plugin::standby()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
void plugin::media_playpause()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
void plugin::media_stop()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
void plugin::media_previous()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
void plugin::volume_relative ( int volume )
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
	sc.setData("volume", volume);
    sendToClients ( sc );
}
void plugin::media_playlist_next ()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
void plugin::media_playlist_previous ()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}

void plugin::media_next()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
void plugin::media_start()
{
    SceneDocument sc;
    sc.setMethod ( __FUNCTION__ );
    sendToClients ( sc );
}
