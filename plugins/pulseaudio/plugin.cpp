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
#include "mediacontroller.h"
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
    if ( IS_ID ( "pulsechannelmute" ) ) {
        set_sink_muted(DATA("sindid").toUtf8().constData(), INTDATA ( "mute" ) );
    } else if ( IS_ID ( "pulsechannelvolume" ) ) {
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

QMap<QString, QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QMap<QString, QVariantMap> l;
    return l;
}

void plugin::pulseSinkChanged ( double volume, bool mute, const QString& sinkid ) {
    PROPERTY ( "pulsechannelstate" );
    data[QLatin1String ( "sinkid" ) ] = sinkid;
    data[QLatin1String ( "mute" ) ] = mute;
    data[QLatin1String ( "volume" ) ] = volume;
    m_server->property_changed ( data );
}

void plugin::pulseVersion(int protocol, int server) {
    PROPERTY ( "pulseversion" );
    data[QLatin1String ( "protocol" ) ] = protocol;
    data[QLatin1String ( "server" ) ] = server;
    m_server->property_changed ( data );
}
