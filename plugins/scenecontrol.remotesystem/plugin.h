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
#include <QHostAddress>
#include "shared/plugins/abstractplugin.h"
#include <QUdpSocket>
#include <QTimer>

class ExternalClient {
public:
    QString host;
    QString identifier;
    int sessionid;
};

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
private:

    QMap<int, ExternalClient> m_clients;
    QStringList m_allowedmembers;
    QVariantMap stateChanged(const ExternalClient* client, bool propagate);
    virtual void session_change(int sessionid, bool running);
	
	void sendToClients(const SceneDocument& sc);
private Q_SLOTS:
    void registerclient(const QString& host, const QString& identifier);
	void mute(int mute);
	void standby();
	void media_start();
	void media_playpause();
	void media_stop();
	void media_next();
	void media_previous();
	void volume_relative(int volume);
	void media_playlist_next();
	void media_playlist_previous();
};
