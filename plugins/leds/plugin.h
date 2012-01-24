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
#include <QTimer>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin();
    virtual ~plugin();

private Q_SLOTS:
    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
    void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);

    // Get, Set Names
    QString getLedName ( const QByteArray& channel );
    void setLedName ( const QByteArray& channel, const QString& name, bool updateDatabase = true );
    // Get, Set Values
    void setLed ( const QByteArray& channel, int value, int fade );
    void setLedExponential ( const QByteArray& channel, int multiplikator, int fade );
    void setLedRelative ( const QByteArray& channel, int value, int fade );
    void toggleLed ( const QByteArray& channel, int fade );
    int getLed( const QByteArray& channel ) const;
    bool isLedValue( const QByteArray& channel, int lower, int upper );
    int countLeds();
    void moodlight(const QByteArray& channel, bool moodlight);

    void moodlightTimeout();
private:
    struct iochannel {
        int value;
        QString name;
        QByteArray channel;
        QByteArray plugin_id;
        bool moodlight;
        int fadeType;

        iochannel() {
            moodlight = false;
            value = -1;
        }
    };
    QMap<QByteArray,iochannel> m_ios;
    QMap<QByteArray, QString> m_namecache;

    QTimer m_moodlightTimer;
};
