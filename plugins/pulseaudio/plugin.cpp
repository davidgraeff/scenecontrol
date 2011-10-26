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
// #include "configplugin.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    //     _config(this);
}

plugin::~plugin() {
    close_pulseaudio();
}

void plugin::clear() {}
void plugin::initialize() {
    reconnect_to_pulse(this);
}


void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
	Q_UNUSED(sessionid);
    if ( ServiceID::isMethod(data, "pulsechannelmute" ) ) {
        set_sink_muted(DATA("sinkid").toUtf8().constData(), INTDATA ( "mute" ) );
    } else if ( ServiceID::isMethod(data, "pulsechannelvolume" ) ) {
        if (BOOLDATA ( "relative" )) {
            set_sink_volume_relative(DATA("sinkid").toUtf8(), DOUBLEDATA ( "volume" ));
        } else {
            set_sink_volume(DATA("sinkid").toUtf8(), DOUBLEDATA ( "volume" ));
        }
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
	Q_UNUSED(sessionid);
	Q_UNUSED(data);
	Q_UNUSED(collectionuid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "pulse.version" );
        sc.setData("protocol", getProtocolVersion());
        sc.setData("server", getServerVersion());
		l.append(sc.getData());
    }
    {
        ServiceCreation sc = ServiceCreation::createModelReset(PLUGIN_ID,  "pulse.channels", "sinkid" );
        l.append(sc.getData());
    }
    QList<PulseChannel> channels = getAllChannels();
    foreach(PulseChannel channel, channels) {
        ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID,  "pulse.channels" );
        sc.setData("sinkid", channel.sinkid);
        sc.setData("mute", channel.mute);
        sc.setData("volume", channel.volume);
        l.append(sc.getData());
    }
    return l;
}

void plugin::pulseSinkChanged ( const PulseChannel& channel ) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID,  "pulse.channels" );
    sc.setData("sinkid", channel.sinkid);
    sc.setData("mute", channel.mute);
    sc.setData("volume", channel.volume);
    m_serverPropertyController->pluginPropertyChanged ( sc.getData() );
}

void plugin::pulseVersion(int protocol, int server) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "pulse.version" );
    sc.setData("protocol", protocol);
    sc.setData("server", server);
    m_serverPropertyController->pluginPropertyChanged ( sc.getData() );
}
