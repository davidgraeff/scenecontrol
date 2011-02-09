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

#pragma once
#include <QObject>
#include <QStringList>
#include <QString>
#include <QVariant>
#include <QMap>
#include <cstdlib> //getenv

class AbstractServiceProvider;
class AbstractStateTracker;
class AbstractPlugin;
class ExecuteWithBase;
class ExecutePlugin : public QObject
{
    Q_OBJECT
public:
    virtual void refresh() = 0;
    virtual ExecuteWithBase* createExecuteService(const QString& id) = 0;
    virtual QList<AbstractStateTracker*> stateTracker() = 0;
    virtual AbstractPlugin* base() = 0;
    /**
     * Called by server process before all services are deleted.
     * Tidy up here.
     */
    virtual void clear() = 0;
    /**
     * Called by server process if a service changed.
     * \param service the changed service
     */
    void serverserviceChanged(AbstractServiceProvider* service);
    /**
     * Called by server process if a setting is changed. Subclass
     * this method (and call the base implementation) to react on
     * changes.
     * \param name Name of the setting
     * \param value Value of the setting
     */
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false);
    virtual void registerSetting(const char* name, const QVariant& value);
    const QVariantMap getSettings() const;
private:
    QVariantMap m_settings;
Q_SIGNALS:
    /**
     * For internal use in plugins only. Server service changes.
     */
    void _serviceChanged(AbstractServiceProvider*);

    void stateChanged(AbstractStateTracker*);

    /**
     * Service will be provided to the server and propagated to the correct plugin.
     * This mechanism allows inter-plugin communication. The handed over service object
     * will get an iExecute flag and be freed by the server after execution.
     */
    void executeService(AbstractServiceProvider*);

    /**
     * Plugin loading finished. This will make the server update all names of this plugins services
     * and redistribute them to the clients (without saving to disk).
     */
    void pluginLoadingComplete(ExecutePlugin*);
};
Q_DECLARE_INTERFACE(ExecutePlugin, "com.roomcontrol.ServerPlugin/1.0")
