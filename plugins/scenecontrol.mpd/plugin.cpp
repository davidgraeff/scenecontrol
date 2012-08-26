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
int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid) {
}

plugin::~plugin() {
}



/*
plugin::plugin(const QString& instanceid) : AbstractPlugin(instanceid) {
    m_mediacontroller = new MediaController ( this );
    connect ( m_mediacontroller,SIGNAL ( playlistChanged ( QString ) ),SLOT ( playlistChanged ( QString ) ) );
    connect ( m_mediacontroller,SIGNAL ( playlistsChanged ( QString,int ) ),SLOT ( playlistsChanged ( QString,int ) ) );
    connect ( m_mediacontroller,SIGNAL ( trackChanged(QString,QString,int,uint,uint,int)), SLOT(trackChanged(QString,QString,int,uint,uint,int)));
    connect ( m_mediacontroller,SIGNAL ( volumeChanged ( double ) ),SLOT ( volumeChanged ( double ) ) );
    connect ( m_mediacontroller,SIGNAL ( stateChanged(MediaController*)),SLOT(stateChanged(MediaController*) ) );
}

plugin::~plugin() {
    delete m_mediacontroller;
}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
  if (data.contains(QLatin1String("host")) && data.contains(QLatin1String("port")))
       m_mediacontroller->connectToMpd ( data[QLatin1String("host")].toString(), data[QLatin1String("port")].toInt());
}

void plugin::execute ( const QVariantMap& data) {
	Q_UNUSED(sessionid);
    if ( ServiceData::isMethod(data, "mpdvolume" ) ) {
        m_mediacontroller->setVolume ( DOUBLEDATA ( "volume" ), BOOLDATA ( "relative" ) );
    } else if ( ServiceData::isMethod(data, "mpdcmd" ) ) {
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
    } else if ( ServiceData::isMethod(data, "mpdchangeplaylist" ) ) {
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

    } else if ( ServiceData::isMethod(data, "mpdposition" ) ) {
        m_mediacontroller->setTrackPosition ( INTDATA ( "position_in_ms" ), BOOLDATA ( "relative" ) );
    }
}

bool plugin::condition ( const QVariantMap& data)  {
	Q_UNUSED(sessionid);
    if ( ServiceData::isMethod(data, "bla" ) ) {
        return ( INTDATA ( "mpdstatecondition" ) == m_mediacontroller->state() );
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid) { 
	Q_UNUSED(sessionid);
    Q_UNUSED ( data );
	Q_UNUSED(collectionuid);
}

void plugin::unregister_event ( const QString& eventid) { 
	Q_UNUSED(sessionid);
	Q_UNUSED(eventid);
}

void plugin::requestProperties(int sessionid) {
    Q_UNUSED(sessionid);

    changeProperty(stateChanged(m_mediacontroller, false));
    {
        ServiceData sc = ServiceData::createNotification(PLUGIN_ID,  "playlist.current" );
        sc.setData("playlistid", m_mediacontroller->currentplaylist());
        changeProperty(sc.getData());
    }
    {
        ServiceData sc = ServiceData::createNotification(PLUGIN_ID,  "volume.changed" );
        sc.setData("volume", m_mediacontroller->volume());
        changeProperty(sc.getData());
    }
    if (m_mediacontroller->state()!=MediaController::NothingLoaded) {
        ServiceData sc = ServiceData::createNotification(PLUGIN_ID,  "track.info" );
		changeProperty(sc.getData());
    }
    {
        ServiceData sc = ServiceData::createModelReset( "playlists", "playlistid" );
        changeProperty(sc.getData());
    }
    return l;
}

void plugin::playlistChanged ( QString p ) {
    ServiceData sc = ServiceData::createNotification(PLUGIN_ID,  "playlist.current" );
    sc.setData("playlistid", p);
    changeProperty ( sc.getData() );
}

void plugin::playlistsChanged ( QString p, int pos) {
    ServiceData sc = ServiceData::createModelChangeItem( "playlists" );
    sc.setData("playlistid", p);
    sc.setData("position", pos);
    changeProperty ( sc.getData() );
}

void plugin::trackChanged ( const QString& filename, const QString& trackname, int track, uint position_in_ms, uint total_in_ms, int state) {
    ServiceData sc = ServiceData::createNotification(PLUGIN_ID,  "track.info" );
    sc.setData("filename", filename);
    sc.setData("trackname", trackname);
    sc.setData("track", track);
    sc.setData("position_in_ms", position_in_ms);
    sc.setData("total_in_ms", total_in_ms);
    sc.setData("state", state);
    changeProperty ( sc.getData() );
}

void plugin::volumeChanged ( double volume ) {
    ServiceData sc = ServiceData::createNotification(PLUGIN_ID,  "volume.changed" );
    sc.setData("volume", volume);
    changeProperty ( sc.getData() );
}

QVariantMap plugin::stateChanged(MediaController* client, bool propagate) {
    ServiceData sc = ServiceData::createNotification(PLUGIN_ID, "connection.state");
    const QString server = client->host()+QLatin1String(":")+QString::number(client->port());
    sc.setData("server",server);
    sc.setData("state", (int)client->isConnected());
    if (propagate) changeProperty(sc.getData());
    return sc.getData();
}*/
