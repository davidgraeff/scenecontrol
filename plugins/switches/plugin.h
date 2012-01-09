/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010-2012  David Gräff
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
#include "shared/abstractserver_collectioncontroller.h"

#include "shared/abstractserver_propertycontroller.h"
#include "shared/pluginservicehelper.h"
#include "shared/abstractplugin_services.h"
#include "shared/plugin_interconnect.h"

class plugin : public QObject, public AbstractPlugin, public AbstractPlugin_services, private PluginInterconnect
{
    Q_OBJECT
    PLUGIN_MACRO
    Q_INTERFACES(AbstractPlugin AbstractPlugin_services)
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual QList<QVariantMap> properties(int sessionid);
    virtual void settingsChanged(const QVariantMap& data);
    virtual void execute ( const QVariantMap& data, int sessionid );
    virtual bool condition ( const QVariantMap& data, int sessionid ) ;
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid );
    virtual void unregister_event ( const QString& eventid, int sessionid );
private:
    virtual dataFromPlugin(const QByteArray& plugin_id, const QByteArray& data);

    QString getSwitchName ( const QString& pin );
    void setSwitch ( const QString& pin, bool value );
    void setSwitchName ( const QString& pin, const QString& name );
    void toggleSwitch ( const QString& pin );
    bool getSwitch( const QString& pin ) const;
    int countSwitchs();

    struct iochannel {
        int value;
        QString name;
        QByteArray plugin_id;

        iochannel() {
            value = 300;
        }
    };
    QMap<QString,iochannel> m_ios;
    QTimer m_cacheTimer;
    QMap<QString, int> m_cache;
private slots:
    void cacheToDevice();
};
