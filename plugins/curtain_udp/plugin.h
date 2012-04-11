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
#include "_sharedsrc/abstractplugin.h"
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
    plugin(const QString& instanceid);
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
    int m_curtainState;
    // udp
    int m_sendPort;
    QUdpSocket *m_socket;
    int m_connectTime;
    QTimer m_connectTimer;
    enum udpcurtain_packet_command {
        udpcurtain_cmd_stop = 250,
        udpcurtain_cmd_start_direction_calibration = 251,
        udpcurtain_cmd_start_direction_calibration_inverted = 252,
        udpcurtain_cmd_direction_ok = 253,
        udpcurtain_cmd_limitsensor_calibration = 254,
        udpcurtain_cmd_request_data = 255
    };
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
    void resendConnectSequence();
public Q_SLOTS:
    void setValue ( int value );
    void limitsensor_calibration();
	void direction_ok();
	void start_direction_calibration();
	void start_direction_calibration_inverted();
	void stop();
    void setRelative ( int value );
    int getValue() const;
    int getMax() const;
    bool isValue(int lower, int upper );
};
