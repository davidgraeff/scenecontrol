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

#pragma once
#include <QObject>
#include <QStringList>
#include "../plugin.h"
#include <QDir>
#include <shared/server/executeplugin.h>

class ModeStateTracker;
class ExecuteWithBase;
class SystemStateTracker;
class BackupStateTracker;
class ExecuteService;
class myPluginExecute : public ExecutePlugin
{
    Q_OBJECT
public:
    myPluginExecute(QObject* parent = 0);
    virtual ~myPluginExecute();
    virtual void refresh() ;
    virtual ExecuteWithBase* createExecuteService(const QString& id);
    virtual QList<AbstractStateTracker*> stateTracker();
	virtual void clear(){}
    virtual AbstractPlugin* base() {
        return m_base;
    }
    void setMode(const QString& mode);
	QString mode() { return m_mode; }
    void backups_changed();
    void backup_create(const QString& name);
	void backup_rename(const QString& id, const QString& name);
    void backup_restore(const QString& id);
    void backup_remove(const QString& id);
    void dataLoadingComplete();
private:
    BackupStateTracker* m_BackupStateTracker;
    SystemStateTracker* m_SystemStateTracker;
    ModeStateTracker* m_ModeStateTracker;
    AbstractPlugin* m_base;
    QString m_mode;
    QDir m_savedir;
};

