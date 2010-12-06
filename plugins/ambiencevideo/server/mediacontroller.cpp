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

#include "mediacontroller.h"
#include <QSettings>
#include <limits>
#include <stdio.h>
#include <qprocess.h>
#include "plugin_server.h"
#include <shared/abstractplugin.h>

MediaController::MediaController(myPluginExecute* plugin) : m_plugin(plugin)
{
    m_playerprocess = new QProcess();
    m_playerprocess->setReadChannel(QProcess::StandardOutput);
    connect (m_playerprocess, SIGNAL(error(QProcess::ProcessError)),SLOT(sloterror(QProcess::ProcessError)));
    connect (m_playerprocess, SIGNAL(finished(int)),SLOT(slotdisconnected(int)));
    connect (m_playerprocess, SIGNAL(started()),SLOT(slotconnected()));
    connect (m_playerprocess, SIGNAL(readyRead()),SLOT(slotreadyRead()));
    connect (m_playerprocess, SIGNAL(readyReadStandardError()),SLOT(readyReadStandardError()));

    slotdisconnected(0);
}

MediaController::~MediaController()
{
	m_playerprocess->blockSignals(true);
    m_playerprocess->terminate();
    if (!m_playerprocess->waitForFinished())
        m_playerprocess->kill();
}


void MediaController::slotconnected()
{
}

void MediaController::slotdisconnected(int)
{
    m_playerprocess->start(QLatin1String("roomvideohelper"));
}

void MediaController::sloterror(QProcess::ProcessError)
{
    qWarning()<<"AmbienceVideo process communication error";
}

void MediaController::readyReadStandardError()
{
    qWarning()<<"AmbienceVideo error:" << m_playerprocess->readAllStandardError().replace('\n',' ');
}

void MediaController::slotreadyRead()
{
    while (m_playerprocess->bytesAvailable())
    {
        QByteArray line = m_playerprocess->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');
    }
}

QList<AbstractStateTracker*> MediaController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    return l;
}

void MediaController::hideVideo() {
    m_playerprocess->write("hideVideo\n");
}
void MediaController::closeFullscreen() {
    m_playerprocess->write("closeFullscreen\n");
}
void MediaController::setFilename(const QString& filename) {
    m_playerprocess->write("play " + filename.toUtf8() +"\n");
}
void MediaController::setVolume(qreal newvol, bool relative) {
    if (relative)
        m_playerprocess->write("volume_relative " + QByteArray::number(newvol*100) + "\n");
    else {
        m_playerprocess->write("volume " + QByteArray::number(newvol*100) + "\n");
    }
}
void MediaController::stop() {
    m_playerprocess->write("stop\n");
}
void MediaController::setDisplay(int display) {
    m_playerprocess->write("display " + QByteArray::number(display) + "\n");
}
void MediaController::setClickActions(ActorAmbienceVideo::EnumOnClick leftclick, ActorAmbienceVideo::EnumOnClick rightclick, int restoretime) {
    m_playerprocess->write("clickactions " + QByteArray::number(leftclick) + " " + QByteArray::number(rightclick) + " " + QByteArray::number(restoretime) + "\n");
}
void MediaController::setDisplayState(int state) {
    m_playerprocess->write("displaystate " + QByteArray::number(state) + "\n");
}
