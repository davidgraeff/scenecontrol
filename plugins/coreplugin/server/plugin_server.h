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
#include "../plugin.h"
#include "executeplugin.h"
#include <QDir>

class EventController;
class EventStateTracker;
class ModeStateTracker;
class ExecuteWithBase;
class SystemStateTracker;
class BackupStateTracker;
class ExecuteService;
class EventVolumeStateTracker;
class myPluginExecute : public ExecutePlugin
{
    Q_OBJECT
public:
    myPluginExecute(QObject* parent = 0);
    virtual ~myPluginExecute();
    virtual void refresh() ;
    virtual ExecuteWithBase* createExecuteService(const QString& id);
    virtual QList<AbstractStateTracker*> stateTracker();
    virtual AbstractPlugin* base() {
        return m_base;
    }
    void setMode(const QString& mode);
    QStringList backups();
    void backup();
    void backup_restore(const QString& id);
    void backup_remove(const QString& id);
    EventController* eventcontroller() {return m_eventcontroller;}
private:
    BackupStateTracker* m_BackupStateTracker;
    SystemStateTracker* m_SystemStateTracker;
    ModeStateTracker* m_ModeStateTracker;
    EventStateTracker* m_EventStateTracker;
    EventVolumeStateTracker* m_EventVolumeStateTracker;
    EventController* m_eventcontroller;
    AbstractPlugin* m_base;
    QString m_mode;
    QDir m_savedir;
private Q_SLOTS:
    void started(const QString& eventTitle, const QString& filename);
    void finished(const QString& eventTitle, const QString& filename);
    void volumeChanged(qreal vol);
};

#endif // COREPLUGINSERVER_H
