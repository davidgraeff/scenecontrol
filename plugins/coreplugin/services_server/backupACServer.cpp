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

#include "backupACServer.h"
#include <coreplugin/services/backupAC.h>
#include <coreplugin/server/plugin_server.h>

ActorBackupServer::ActorBackupServer(ActorBackup* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_base(base), m_plugin(plugin) {}
bool ActorBackupServer::checkcondition() {
	return true;
}
void ActorBackupServer::execute() {
	switch (m_base->action()) {
		case ActorBackup::CreateBackup:
			m_plugin->backup_create(m_base->backupname());
			break;
		case ActorBackup::RemoveBackup:
			m_plugin->backup_remove(m_base->backupid());
			break;
		case ActorBackup::RestoreBackup:
			m_plugin->backup_restore(m_base->backupid());
			break;
		case ActorBackup::RenameBackup:
			m_plugin->backup_rename(m_base->backupid(), m_base->backupname());
			break;
		default:
			break;
	};
}
void ActorBackupServer::dataUpdate() {}

void ActorBackupServer::nameUpdate() {
	m_base->setString(tr("Backup %1").arg(m_base->translate(0,m_base->action())));
}
