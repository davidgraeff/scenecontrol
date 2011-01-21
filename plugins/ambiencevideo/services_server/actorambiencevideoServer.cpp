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

#include "actorambiencevideoServer.h"
#include "server/plugin_server.h"
#include "server/mediacontroller.h"
#include <services/actorambiencevideo.h>

void ActorAmbienceVideoServer::execute()
{
    ActorAmbienceVideo* base = service<ActorAmbienceVideo>();
    m_plugin->mediacontroller()->setDisplay(base->display());
    m_plugin->mediacontroller()->setClickActions(base->onleftclick(),base->onrightclick(), base->restoretime());
    m_plugin->mediacontroller()->setVolume(base->volume());
    m_plugin->mediacontroller()->setFilename(base->filename());
}

ActorAmbienceVideoServer::ActorAmbienceVideoServer(ActorAmbienceVideo* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {}
void ActorAmbienceVideoServer::nameUpdate() {
	ActorAmbienceVideo* base = service<ActorAmbienceVideo>();
	base->setString(tr("Ambience Video\n%1").arg(base->filename()));
}
