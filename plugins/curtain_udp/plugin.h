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
#include "shared/abstractplugin.h"
#include <QStringList>
#include <QObject>
#include <QMap>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
private:
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
    void connectToServer(const QString& host, int port);
    void curtainChanged(int sessionid=-1);
    int m_curtainvalue;
    int m_curtainmax;
    int m_curtainButtons;
    // udp
    int m_sendPort;
    QUdpSocket *m_socket;
    int m_connectTime;
    QTimer m_connectTimer;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
    void resendConnectSequence();
public Q_SLOTS:
    void setCurtain ( int value );
    void syncCurtain();
    void setCurtainRelative ( int value );
    int getCurtain() const;
    int getCurtainMax() const;
    bool isCurtainValue(int lower, int upper );
};
