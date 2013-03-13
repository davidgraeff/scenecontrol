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
#include "abstractplugin.h"
#include <QTimer>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:

    virtual ~plugin();
private Q_SLOTS:

    virtual void initialize();
    virtual void clear();
    void clear(const QString& componentid_, const QString& instanceid_);
    virtual void requestProperties();
    virtual void instanceConfiguration(const QVariantMap& data);
    // Call this method to update <channel, value>-pairs and clear(your_plugin_id) to clear
    void subpluginChange( const QString& componentid_, const QString& instanceid_, const QString& channel, bool value, const QString& name );

    // Get, Set Names
    QString getSwitchName ( const QString& channel );
    void setSwitchName ( const QString& channel, const QString& name );
    // Get, Set Values
    void setSwitch ( const QString& channel, bool value );
    void toggleSwitch ( const QString& channel );
    bool getSwitch( const QString& channel ) const;
    bool isSwitchOn( const QString& channel, bool value );
    int countSwitchs();
private:
    struct iochannel {
        bool value;
        QString name;
		QString channel;
        QByteArray componentID;
		QByteArray instanceID;

        iochannel() {
            value = -1;
        }
    };
    QMap<QString,iochannel> m_ios;
    QSet<iochannel*> m_cache;
    QMap<QString, QString> m_namecache;
};
