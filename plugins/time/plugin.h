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
#define PLUGIN_ID "bla"
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QSet>
#include "shared/abstractplugin.h"
#include "shared/abstractserver.h"
#include "shared/pluginhelper.h"

class plugin : public QObject, public PluginHelper
{
    Q_OBJECT
    PLUGIN_MACRO
    Q_INTERFACES(AbstractPlugin)
public:
    plugin();
    virtual ~plugin();

    virtual void init(AbstractServer* server);
    virtual QMap<QString, QVariantMap> properties();
    virtual void clear();
    virtual void otherPropertyChanged(const QString& unqiue_property_id, const QVariantMap& value);
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false);
    virtual void execute(const QVariantMap& data);
    virtual bool condition(const QVariantMap& data) ;
    virtual void event_changed(const QVariantMap& data);
private:
	void calculate_next_events();
	QMap<QString, QVariantMap> m_remaining_events;
	QSet<QString> m_timeout_service_ids;
	AbstractServer* m_server;
	QTimer m_timer;
private Q_SLOTS:
	void timeout();
};
