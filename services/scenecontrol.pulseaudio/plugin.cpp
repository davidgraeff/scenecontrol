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
#include "pulseController.h"
#include <QCoreApplication>

/**
 * A plugin is a separate process and for that reason a main function have to be implemented
 * which instantiate the plugin object.
 */
int main ( int argc, char* argv[] )
{
    QCoreApplication app ( argc, argv );
    if (argc<4) {
    {
        qWarning()<<"Usage: instance_id server_ip server_port";
        return 1;
    }
    
    if (plugin::createInstance(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 10;
    return app.exec();
}

plugin::plugin ( const QString& pluginid, const QString& instanceid ) : AbstractPlugin ( pluginid, instanceid )
{
}

plugin::~plugin()
{
    close_pulseaudio();
}

void plugin::initialize()
{
    reconnect_to_pulse ( this );
}

void plugin::requestProperties ( int sessionid )
{
    Q_UNUSED ( sessionid );

    {
        SceneDocument sc = SceneDocument::createNotification ( "pulse.version" );
        sc.setData ( "protocol", getProtocolVersion() );
        sc.setData ( "server", getServerVersion() );
        changeProperty ( sc.getData() );
    }
    {
        SceneDocument sc = SceneDocument::createModelReset ( "pulse.channels", "sinkid" );
        changeProperty ( sc.getData() );
    }
    QList<PulseChannel> channels = getAllChannels();
    foreach ( PulseChannel channel, channels )
    {
        SceneDocument sc = SceneDocument::createModelChangeItem ( "pulse.channels" );
        sc.setData ( "sinkid", channel.sinkid );
        sc.setData ( "mute", channel.mute );
        sc.setData ( "volume", channel.volume );
        changeProperty ( sc.getData() );
    }
    return l;
}

void plugin::pulseSinkChanged ( const PulseChannel& channel )
{
    SceneDocument sc = SceneDocument::createModelChangeItem ( "pulse.channels" );
    sc.setData ( "sinkid", channel.sinkid );
    sc.setData ( "mute", channel.mute );
    sc.setData ( "volume", channel.volume );
    changeProperty ( sc.getData() );
}

void plugin::pulseVersion ( int protocol, int server )
{
    SceneDocument sc = SceneDocument::createNotification ( "pulse.version" );
    sc.setData ( "protocol", protocol );
    sc.setData ( "server", server );
    changeProperty ( sc.getData() );
}

void plugin::pulsechannelmute ( const QByteArray& sinkid, bool mute )
{
    set_sink_muted ( sinkid, mute );
}

void plugin::pulsechannelmutetoggle ( const QByteArray& sinkid )
{
	set_sink_toggle_muted(sinkid);
}

void plugin::pulsechannelvolume ( const QByteArray& sinkid, double volume, bool relative )
{
    if ( relative )
    {
        set_sink_volume_relative ( sinkid, volume );
    }
    else
    {
        set_sink_volume ( sinkid, volume );
    }
}
