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
#include "mediacontroller.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_mediacontroller = new MediaController ( this );
    connect ( m_mediacontroller,SIGNAL ( playlistChanged ( QString ) ),SLOT ( playlistChanged ( QString ) ) );
    connect ( m_mediacontroller,SIGNAL ( playlistsChanged ( QString,int ) ),SLOT ( playlistsChanged ( QString,int ) ) );
    connect ( m_mediacontroller,SIGNAL ( trackChanged(QString,QString,int,uint,uint,int)), SLOT(trackChanged(QString,QString,int,uint,uint,int)));
    connect ( m_mediacontroller,SIGNAL ( volumeChanged ( double ) ),SLOT ( volumeChanged ( double ) ) );
    connect ( m_mediacontroller,SIGNAL ( stateChanged(MediaController*)),SLOT(stateChanged(MediaController*) ) );
    _config ( this );
}

plugin::~plugin() {
    delete m_mediacontroller;
}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if ( name == QLatin1String ( "server" ) ) {
        m_mediacontroller->connectToMpd ( value.toString() );
    }
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
	Q_UNUSED(sessionid);
    if ( ServiceID::isMethod(data, "mpdvolume" ) ) {
        m_mediacontroller->setVolume ( DOUBLEDATA ( "volume" ), BOOLDATA ( "relative" ) );
    } else if ( ServiceID::isMethod(data, "mpdcmd" ) ) {
        switch ( INTDATA ( "state" ) ) {
        case 0:
            m_mediacontroller->play();
            break;
        case 1:
            m_mediacontroller->pause();
            break;
        case 2:
            m_mediacontroller->stop();
            break;
        case 3:
            m_mediacontroller->next();
            break;
        case 4:
            m_mediacontroller->previous();
            break;
        case 5:
            m_mediacontroller->nextPlaylist();
            break;
        case 6:
            m_mediacontroller->previousPlaylist();
            break;
        case 7:
            m_mediacontroller->dumpMediaInfo();
            break;
        default:
            break;
        }
    } else if ( ServiceID::isMethod(data, "mpdchangeplaylist" ) ) {
        // set playlist
        const QString playlistid = DATA ( "playlistid" );
        const int track = INTDATA ( "track" );
        if ( playlistid.size() ) {
            m_mediacontroller->setPlaylist ( playlistid );
        }
        // set track number
        if ( track != -1 ) {
            m_mediacontroller->setCurrentTrack ( track );
        }

    } else if ( ServiceID::isMethod(data, "mpdposition" ) ) {
        m_mediacontroller->setTrackPosition ( INTDATA ( "position_in_ms" ), BOOLDATA ( "relative" ) );
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
	Q_UNUSED(sessionid);
    if ( ServiceID::isMethod(data, "bla" ) ) {
        return ( INTDATA ( "mpdstatecondition" ) == m_mediacontroller->state() );
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) { 
	Q_UNUSED(sessionid);
    Q_UNUSED ( data );
	Q_UNUSED(collectionuid);
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) { 
	Q_UNUSED(sessionid);
	Q_UNUSED(data);
	Q_UNUSED(collectionuid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    l.append(stateChanged(m_mediacontroller, false));
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "playlist.current" );
        sc.setData("playlistid", m_mediacontroller->currentplaylist());
        l.append(sc.getData());
    }
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "volume.changed" );
        sc.setData("volume", m_mediacontroller->volume());
        l.append(sc.getData());
    }
    if (m_mediacontroller->state()!=MediaController::NothingLoaded) {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "track.info" );
		l.append(sc.getData());
    }
    {
        ServiceCreation sc = ServiceCreation::createModelReset(PLUGIN_ID,  "playlists", "playlistid" );
        l.append(sc.getData());
    }
    return l;
}

void plugin::playlistChanged ( QString p ) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "playlist.current" );
    sc.setData("playlistid", p);
    m_server->pluginPropertyChanged ( sc.getData() );
}

void plugin::playlistsChanged ( QString p, int pos) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID,  "playlists" );
    sc.setData("playlistid", p);
    sc.setData("position", pos);
    m_server->pluginPropertyChanged ( sc.getData() );
}

void plugin::trackChanged ( const QString& filename, const QString& trackname, int track, uint position_in_ms, uint total_in_ms, int state) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "track.info" );
    sc.setData("filename", filename);
    sc.setData("trackname", trackname);
    sc.setData("track", track);
    sc.setData("position_in_ms", position_in_ms);
    sc.setData("total_in_ms", total_in_ms);
    sc.setData("state", state);
    m_server->pluginPropertyChanged ( sc.getData() );
}

void plugin::volumeChanged ( double volume ) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "volume.changed" );
    sc.setData("volume", volume);
    m_server->pluginPropertyChanged ( sc.getData() );
}

QVariantMap plugin::stateChanged(MediaController* client, bool propagate) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "connection.state");
    const QString server = client->host()+QLatin1String(":")+QString::number(client->port());
    sc.setData("server",server);
    sc.setData("state", (int)client->isConnected());
    if (propagate) m_server->pluginPropertyChanged(sc.getData());
    return sc.getData();
}
