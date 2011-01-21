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

#include "modeCOServer.h"
#include "coreplugin/server/plugin_server.h"
#include "coreplugin/services/modeCO.h"

ConditionModeServer::ConditionModeServer(ConditionMode* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_base(base), m_plugin(plugin) {
}
bool ConditionModeServer::checkcondition() {
	return (m_base->mode()==m_plugin->mode());
}
void ConditionModeServer::execute() {
}
void ConditionModeServer::dataUpdate() {}

void ConditionModeServer::nameUpdate() {
	ConditionMode* base = service<ConditionMode>();
	
	base->setString(tr("Modus ist %1").arg(base->mode()));
}
