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

#include <qprocess.h>
#include <services/actorambiencevideo.h>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QUrl>

class AbstractStateTracker;
class ExecuteWithBase;
class myPluginExecute;
class AbstractServiceProvider;

class ExternalClient : public QTcpSocket
{
    Q_OBJECT
public:
    ExternalClient(myPluginExecute* plugin, const QString& host, int port);
    ~ExternalClient();
    QList<AbstractStateTracker*> getStateTracker();

    void hideVideo();
    void closeFullscreen();
    void setFilename(const QString& filename);
    void stopvideo();
    void setVolume(qreal newvol, bool relative = false);
    void setDisplay(int display);
    void setClickActions(ActorAmbienceVideo::EnumOnClick leftclick, ActorAmbienceVideo::EnumOnClick rightclick, int restoretime);
    void setDisplayState(int state);
    void stopevent();

    void showMessage(int duration, const QString& msg);
    void playEvent(const QString& filename);
    void setVolumeEvent(qreal newvol, bool relative = false);
private:
    myPluginExecute* m_plugin;
    QTimer m_reconnect;
    QString m_host;
    int m_port;
    int m_display;
    bool m_alreadyWarnedNoHost;
private Q_SLOTS:
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected();
    void sloterror(QAbstractSocket::SocketError);
    void reconnectTimeout();
};
