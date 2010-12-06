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

#include <QTcpSocket>
#include <qprocess.h>
#include <services/actorambiencevideo.h>

class AbstractStateTracker;
class ExecuteWithBase;
class myPluginExecute;
class AbstractServiceProvider;

class MediaController : public QObject
{
    Q_OBJECT
public:
    MediaController(myPluginExecute* plugin);
    ~MediaController();
    QList<AbstractStateTracker*> getStateTracker();

    void hideVideo();
    void closeFullscreen();
    void setFilename(const QString& filename);
	void setVolume(qreal newvol, bool relative = false);
    void setDisplay(int display);
	void setClickActions(ActorAmbienceVideo::EnumOnClick leftclick, ActorAmbienceVideo::EnumOnClick rightclick, int restoretime);
    void setDisplayState(int state);
	void stop();

private:
    myPluginExecute* m_plugin;
    QProcess* m_playerprocess;
	int m_display;
private Q_SLOTS:
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected(int);
    void sloterror(QProcess::ProcessError e);
    void readyReadStandardError() ;
Q_SIGNALS:
    void stateChanged(AbstractStateTracker*);
};
