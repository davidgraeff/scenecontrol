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
#include "shared/plugins/abstractplugin.h"
#include <QTimer>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin(const QString& pluginid, const QString& instanceid);
    virtual ~plugin();

private Q_SLOTS:
    virtual void initialize();
    virtual void clear();
    void clear(const QByteArray& plugin_);
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
    // Call this method to update <channel, value>-pairs and clear(your_plugin_id) to clear
	void subpluginChange(const QString& componentid_, const QString& instanceid_, const QString& channel, int value, const QString& name);

    // Get, Set Names
    QString getLedName ( const QString& channel );
    void setLedName ( const QString& channel, const QString& name, bool updateDatabase = true );
    // Get, Set Values
    void setLed ( const QString& channel, int value, int fade );
    void setLedExponential ( const QString& channel, int multiplikator, int fade );
    void setLedRelative ( const QString& channel, int value, int fade );
    void toggleLed ( const QString& channel, int fade );
    int getLed( const QString& channel ) const;
    bool isLedValue( const QString& channel, int lower, int upper );
    int countLeds();
    void moodlight(const QString& channel, bool moodlight);

    void moodlightTimeout();
private:
    struct iochannel {
        int value;
        QString name;
        QString channel;
        QByteArray componentUniqueID;
        bool moodlight;
        int fadeType;

        iochannel() {
            moodlight = false;
            value = -1;
        }
    };
    QMap<QString,iochannel> m_ios;
    QMap<QString, QString> m_namecache;

    QTimer m_moodlightTimer;
};
