/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once
#include <QAbstractListModel>
#include <QStringList>
#include <QUuid>
#include <QTimer>

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QUrl>

class AbstractPlugin;
class ExternalClient : public QTcpSocket
{
    Q_OBJECT
public:
    ExternalClient(AbstractPlugin* plugin, const QString& host, int port);
    ~ExternalClient();
	const QString host() const { return m_host; }
	int port() const { return m_port; }
	int isConnected() const { return m_connected; }

    void setSystemVolume(qreal newvol, bool relative = false);
    void setDisplayState(int state, int display);
    void stopevent();

    void showVideo(const QString& filename, int display);
    void showMessage(int duration, const QString& msg, const QString& audiofile);
private:
    AbstractPlugin* m_plugin;
    QTimer m_reconnect;
    QString m_host;
    int m_port;
    int m_display;
    bool m_connected;
private Q_SLOTS:
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected();
    void sloterror(QAbstractSocket::SocketError);
    void reconnectTimeout();
Q_SIGNALS:
	void stateChanged(ExternalClient* client);
};
