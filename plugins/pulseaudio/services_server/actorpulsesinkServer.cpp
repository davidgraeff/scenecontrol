/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "actorpulsesinkServer.h"
#include <services/actorpulsesink.h>
#include "server/plugin_server.h"
#include "server/mediacontroller.h"

void ActorPulseSinkServer::execute()
{
    ActorPulseSink* base = service<ActorPulseSink>();
    if (base->mute()==ActorPulseSink::MuteSink)
        m_plugin->mediacontroller()->setPAMute(base->sinkid().toAscii(),1);
    else if (base->mute()==ActorPulseSink::UnmuteSink)
        m_plugin->mediacontroller()->setPAMute(base->sinkid().toAscii(),0);
    else if (base->mute()==ActorPulseSink::ToggleSink)
        m_plugin->mediacontroller()->togglePAMute(base->sinkid().toAscii());

    if (base->assignment()==ActorPulseSink::NoVolumeSet) return;
    m_plugin->mediacontroller()->setPAVolume(base->sinkid().toAscii(),base->volume(),(base->assignment()==ActorPulseSink::VolumeRelative));
}

ActorPulseSinkServer::ActorPulseSinkServer(ActorPulseSink* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}

void ActorPulseSinkServer::nameUpdate() {
    ActorPulseSink* base = service<ActorPulseSink>();

    if (base->assignment()==ActorPulseSink::NoVolumeSet) {
        base->setString(tr("Pulseaudiosink \"%1\"\nMute: %2").arg(base->sinkid()).arg(base->translate(1,base->mute())));
    } else if (base->assignment()==ActorPulseSink::VolumeAbsolute) {
		base->setString(tr("Pulseaudiosink \"%1\"\nVolume: %2, Mute: %3").arg(base->sinkid()).arg(base->volume()).arg(base->translate(1,base->mute()))  );
	} else if (base->assignment()==ActorPulseSink::VolumeRelative) {
		base->setString(tr("Pulseaudiosink \"%1\"\n+/- Volume: %2, Mute: %3").arg(base->sinkid()).arg(base->volume()).arg(base->translate(1,base->mute()))  );
    }
}
