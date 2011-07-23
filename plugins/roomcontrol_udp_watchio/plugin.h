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
#include "shared/abstractplugin.h"
#include "shared/abstractserver.h"
#include "shared/pluginsettingshelper.h" 
#include "shared/pluginservicehelper.h"
#include "shared/abstractplugin_services.h"
#include <QUdpSocket>

class Controller;
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
    virtual QList<QVariantMap> properties(const QString& sessionid);
    virtual void setSetting ( const QString& name, const QVariant& value, bool init = false );
    virtual void execute ( const QVariantMap& data, const QString& sessionid );
    virtual bool condition ( const QVariantMap& data, const QString& sessionid ) ;
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid );
	virtual void unregister_event ( const QVariantMap& data, const QString& collectionuid );
private:
    EventMap<int> m_events; //mode->set of uids
    QUdpSocket *m_socket;
	QVector<bool> m_sensors;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
};