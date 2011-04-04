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

void plugin::execute ( const QVariantMap& data ) {
    if ( IS_ID ( "mpdvolume" ) ) {
        m_mediacontroller->setVolume ( DOUBLEDATA ( "volume" ), BOOLDATA ( "relative" ) );
    } else if ( IS_ID ( "mpdcmd" ) ) {
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
    } else if ( IS_ID ( "mpdchangeplaylist" ) ) {
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

    } else if ( IS_ID ( "mpdposition" ) ) {
        m_mediacontroller->setTrackPosition ( INTDATA ( "position_in_ms" ), BOOLDATA ( "relative" ) );
    }
}

bool plugin::condition ( const QVariantMap& data )  {
    if ( IS_ID ( "bla" ) ) {
        return ( INTDATA ( "mpdstatecondition" ) == m_mediacontroller->state() );
    }
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    l.append(stateChanged(m_mediacontroller, false));
    return l;
}

void plugin::playlistChanged ( QString p ) {
    PROPERTY ( "mpdplaylist" );
    data[QLatin1String ( "playlistid" ) ] = p;
    m_server->property_changed ( data );
}

void plugin::playlistsChanged ( QString p, int pos) {
    PROPERTY ( "mpdplaylists" );
    data[QLatin1String ( "playlistid" ) ] = p;
    data[QLatin1String ( "position" ) ] = pos;
    m_server->property_changed ( data );
}

void plugin::trackChanged ( const QString& filename, const QString& trackname, int track, uint position_in_ms, uint total_in_ms, int state) {
    PROPERTY ( "mpdtrack" );
    data[QLatin1String ( "filename" ) ] = filename;
    data[QLatin1String ( "trackname" ) ] = trackname;
    data[QLatin1String ( "track" ) ] = track;
    data[QLatin1String ( "position_in_ms" ) ] = position_in_ms;
    data[QLatin1String ( "total_in_ms" ) ] = total_in_ms;
    data[QLatin1String ( "state" ) ] = state;
    m_server->property_changed ( data );
}

void plugin::volumeChanged ( double volume ) {
    PROPERTY ( "mpdvolumechanged" );
    data[QLatin1String ( "volume" ) ] = volume;
    m_server->property_changed ( data );
}

QVariantMap plugin::stateChanged(MediaController* client, bool propagate) {
    PROPERTY("mpd.connection.state");
	const QString server = client->host()+QLatin1String(":")+QString::number(client->port());
    SETDATA("server",server);
    SETDATA("state", (int)client->isConnected());
    if (propagate) m_server->property_changed(data);
    return data;
}
