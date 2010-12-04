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

#include "modeEVServer.h"
#include "coreplugin/services/modeEV.h"
#include "coreplugin/server/plugin_server.h"

EventModeServer::EventModeServer(EventMode* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_base(base), m_plugin(plugin) {
	connect(plugin,SIGNAL(modeChanged()),SLOT(modeChanged()));
}
bool EventModeServer::checkcondition() {
	return true;
}
void EventModeServer::execute() {
}
void EventModeServer::dataUpdate() {}

void EventModeServer::modeChanged() {
	if (m_base->mode().isEmpty() || m_base->mode()==m_plugin->mode()) {
		emit trigger();
	}
	
}
