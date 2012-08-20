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
#include "shared/plugins/abstractplugin.h"
#include <QUdpSocket>
#include <QTimer>

class Controller;
class plugin : public AbstractPlugin
{
    Q_OBJECT


public:
    plugin(const QString& pluginid, const QString& instanceid);
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
public Q_SLOTS:
    void setSwitch ( const QString& channel, bool value);
    void toggleSwitch ( const QString& channel );
    bool getSwitch( const QString& channel ) const;
    bool isSwitchOn( const QString& channel, bool value );
    int countSwitchs();
    void connectToIOs(int portSend, int portListen, const QString& user, const QString& pwd);
private:
    QMap< QString, QPair<QHostAddress,uint> > m_mapChannelToHost;
    int m_sendPort;
    QString m_user;
    QString m_pwd;
    QUdpSocket *m_listenSocket;
    QUdpSocket *m_writesocket;
    QTimer m_cacheTimer;
    // Host address -> 8-bit value (= 8 switches)
    QMap<QString, unsigned char> m_cache;

    struct iochannel {
        bool value;
        iochannel() {
            value = -1;
        }
    };
    QMap<QString,iochannel> m_ios;
private Q_SLOTS:
    void readyRead();
    void cacheToDevice();
};
