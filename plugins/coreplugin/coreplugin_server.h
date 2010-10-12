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

#ifndef COREPLUGINSERVER_H
#define COREPLUGINSERVER_H
#include <QObject>
#include <QStringList>
#include "coreplugin.h"
#include "../../server/executeplugin.h"

class ModeStateTracker;
class ExecuteWithBase;
class SystemStateTracker;
class BackupStateTracker;
class ExecuteService;
class ServiceController;
class CorePluginExecute : public ExecutePlugin
{
	Q_OBJECT
public:
	CorePluginExecute(ServiceController* services);
	virtual ~CorePluginExecute();
	virtual void refresh() ;
	virtual ExecuteWithBase* createExecuteService(const QString& id);
	virtual QList<AbstractStateTracker*> stateTracker();
	ServiceController* serviceController() ;
    void setMode(const QString& mode);
	QStringList backups();
	void backup();
	void backup_restore(const QString& id);
	void backup_remove(const QString& id);
private:
	ServiceController* m_services;
	BackupStateTracker* m_BackupStateTracker;
	SystemStateTracker* m_SystemStateTracker;
	ModeStateTracker* m_ModeStateTracker;
	// Backups
Q_SIGNALS:
	void modeChanged();
};

#endif // COREPLUGINSERVER_H
