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

void plugin::execute ( const QVariantMap& data ) {
    if ( ServiceID::isId(data, "pulsechannelmute" ) ) {
        set_sink_muted(DATA("sindid").toUtf8().constData(), INTDATA ( "mute" ) );
    } else if ( ServiceID::isId(data, "pulsechannelvolume" ) ) {
        if (BOOLDATA ( "relative" )) {
            set_sink_volume_relative(DATA("sindid").toUtf8(), DOUBLEDATA ( "volume" ));
        } else {
            set_sink_volume(DATA("sindid").toUtf8(), DOUBLEDATA ( "volume" ));
        }
    }
}

bool plugin::condition ( const QVariantMap& data )  {
    Q_UNUSED ( data );
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
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
    m_server->property_changed ( sc.getData() );
}

void plugin::pulseVersion(int protocol, int server) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID,  "pulse.version" );
    sc.setData("protocol", protocol);
    sc.setData("server", server);
    m_server->property_changed ( sc.getData() );
}
