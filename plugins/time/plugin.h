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
#include <QTimer>
#include <QSet>
#include "shared/abstractplugin.h"
#include "shared/abstractserver.h"
#include "shared/pluginsettingshelper.h" 
#include "shared/pluginservicehelper.h"
#include "shared/abstractplugin_services.h"
#include <QDateTime>

class plugin : public QObject, public PluginSettingsHelper, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
    Q_INTERFACES(AbstractPlugin AbstractPlugin_settings AbstractPlugin_services)
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
	virtual void clear();
    virtual QList<QVariantMap> properties(int sessionid);
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false);
    virtual void execute(const QVariantMap& data, int sessionid);
    virtual bool condition(const QVariantMap& data, int sessionid) ;
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid );
	virtual void unregister_event ( const QVariantMap& data, const QString& collectionuid, int sessionid );
private:
	void calculate_next_events();
	typedef QPair<QString, QString> SC_Uid;
	typedef QMap<SC_Uid, QVariantMap> DataBySC_Uid;
	QMap<QString, QVariantMap> m_remaining_events;
	DataBySC_Uid m_timeout_events;
	QTimer m_timer;
	QDateTime m_nextAlarm;
private Q_SLOTS:
	void timeout();
};
