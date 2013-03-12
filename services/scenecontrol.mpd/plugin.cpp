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
#include "mpdController.h"

/**
 * A plugin is a separate process and for that reason a main function have to be implemented
 * which instantiate the plugin object.
 */
int main ( int argc, char* argv[] )
{
    QCoreApplication app ( argc, argv );
    if (argc<4) {
        qWarning()<<"Usage: instance_id server_ip server_port";
        return 1;
    }
    
    if (plugin::createInstance(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 10;
    return app.exec();
}

plugin::~plugin()
{
    delete m_mediacontroller;
}


void plugin::configChanged ( const QByteArray& configid, const QVariantMap& data )
{
	Q_UNUSED ( configid );
    if ( data.contains ( QLatin1String ( "host" ) ) && data.contains ( QLatin1String ( "port" ) ) )
        m_mediacontroller->connectToMpd ( data[QLatin1String ( "host" )].toString(), data[QLatin1String ( "port" )].toInt() );
}

void plugin::requestProperties ( int sessionid )
{
    Q_UNUSED ( sessionid );

    changeProperty ( stateChanged ( m_mediacontroller, false ) );
    {
        SceneDocument sc = SceneDocument::createNotification ("mpd.playlist.current" );
        sc.setData ( "playlistid", m_mediacontroller->currentplaylist() );
        changeProperty ( sc.getData() );
    }
    {
        SceneDocument sc = SceneDocument::createNotification ("mpd.volume.changed" );
        sc.setData ( "volume", m_mediacontroller->volume() );
        changeProperty ( sc.getData() );
    }
    if ( m_mediacontroller->state() !=MediaController::NothingLoaded )
    {
        SceneDocument sc = SceneDocument::createNotification ("mpd.track.info" );
        changeProperty ( sc.getData() );
    }
    {
        SceneDocument sc = SceneDocument::createModelReset ( "mpd.playlists", "playlistid" );
        changeProperty ( sc.getData() );
    }
}

void plugin::playlistChanged ( QString p )
{
    SceneDocument sc = SceneDocument::createNotification ("mpd.playlist.current" );
    sc.setData ( "playlistid", p );
    changeProperty ( sc.getData() );
}

void plugin::playlistsChanged ( QString p, int pos )
{
    SceneDocument sc = SceneDocument::createModelChangeItem ( "mpd.playlists" );
    sc.setData ( "playlistid", p );
    sc.setData ( "position", pos );
    changeProperty ( sc.getData() );
}

void plugin::trackChanged ( const QString& filename, const QString& trackname, int track, uint position_in_ms, uint total_in_ms, int state )
{
    SceneDocument sc = SceneDocument::createNotification ("mpd.track.info" );
    sc.setData ( "filename", filename );
    sc.setData ( "trackname", trackname );
    sc.setData ( "track", track );
    sc.setData ( "position_in_ms", position_in_ms );
    sc.setData ( "total_in_ms", total_in_ms );
    sc.setData ( "state", state );
    changeProperty ( sc.getData() );
}

void plugin::volumeChanged ( double volume )
{
    SceneDocument sc = SceneDocument::createNotification ("mpd.volume.changed" );
    sc.setData ( "volume", volume );
    changeProperty ( sc.getData() );
}

QVariantMap plugin::stateChanged ( MediaController* client, bool propagate )
{
    SceneDocument sc = SceneDocument::createNotification ( "mpd.connection.state" );
    const QString server = client->host() +QLatin1String ( ":" ) +QString::number ( client->port() );
    sc.setData ( "server",server );
    sc.setData ( "state", ( int ) client->isConnected() );
    if ( propagate ) changeProperty ( sc.getData() );
    return sc.getData();
}

void plugin::mpdvolume ( double volume, bool relative )
{
    m_mediacontroller->setVolume ( volume, relative );
}

void plugin::mpdposition ( int position_in_ms, bool relative )
{
    m_mediacontroller->setTrackPosition ( position_in_ms, relative );
}

void plugin::play()
{
    m_mediacontroller->play();
}

void plugin::pause()
{
    m_mediacontroller->pause();
}

void plugin::stop()
{
    m_mediacontroller->stop();
}

void plugin::next()
{
    m_mediacontroller->next();
}

void plugin::previous()
{
    m_mediacontroller->previous();
}

void plugin::nextPlaylist()
{
    m_mediacontroller->nextPlaylist();
}

void plugin::previousPlaylist()
{
    m_mediacontroller->previousPlaylist();
}

void plugin::dumpMediaInfo()
{
    m_mediacontroller->dumpMediaInfo();
}

void plugin::mpdchangeplaylist ( const QString& playlistid, int track )
{
    // set playlist
    if ( playlistid.size() )
    {
        m_mediacontroller->setPlaylist ( playlistid );
    }
    // set track number
    if ( track != -1 )
    {
        m_mediacontroller->setCurrentTrack ( track );
    }
}
void plugin::initialize() {
	m_mediacontroller = new MediaController ( this );
	connect ( m_mediacontroller,SIGNAL ( playlistChanged ( QString ) ),SLOT ( playlistChanged ( QString ) ) );
	connect ( m_mediacontroller,SIGNAL ( playlistsChanged ( QString,int ) ),SLOT ( playlistsChanged ( QString,int ) ) );
	connect ( m_mediacontroller,SIGNAL ( trackChanged ( QString,QString,int,uint,uint,int ) ), SLOT ( trackChanged ( QString,QString,int,uint,uint,int ) ) );
	connect ( m_mediacontroller,SIGNAL ( volumeChanged ( double ) ),SLOT ( volumeChanged ( double ) ) );
	connect ( m_mediacontroller,SIGNAL ( stateChanged ( MediaController* ) ),SLOT ( stateChanged ( MediaController* ) ) );
}
