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

#include "profileACServer.h"
#include <coreplugin/services/profileAC.h>
#include <coreplugin/server/plugin_server.h>
#include <../server/servicecontroller.h>

ActorCollectionServer::ActorCollectionServer(ActorCollection* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_base(base), m_plugin(plugin) {}
bool ActorCollectionServer::checkcondition() {
	return true;
}
void ActorCollectionServer::execute() {
	
	switch (m_base->action()) {
		case ActorCollection::StartProfile:
			m_plugin->serviceController()->runProfile(m_base->profileid());
			break;
		case ActorCollection::CancelProfile:
			m_plugin->serviceController()->stopProfile(m_base->profileid());
			break;
		default:
			break;
	};
}
void ActorCollectionServer::dataUpdate() {}
