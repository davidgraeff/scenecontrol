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

#include "externalclient.h"
#include <QSettings>
#include <limits>
#include <stdio.h>
#include <qprocess.h>
#include "plugin_server.h"
#include <shared/abstractplugin.h>

ExternalClient::ExternalClient(myPluginExecute* plugin, const QUrl& address) : m_plugin(plugin), m_address(address), m_alreadyWarnedNoHost(false)
{
    connect(&m_reconnect,SIGNAL(timeout()),SLOT(reconnectTimeout()));
    connect(this, SIGNAL(connected()), SLOT(slotconnected()));
    connect(this, SIGNAL(disconnected()), SLOT(slotdisconnected()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(sloterror(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(readyRead()), SLOT(slotreadyRead()));
    m_reconnect.setInterval(60000);
    m_reconnect.start();
}

ExternalClient::~ExternalClient()
{
    disconnectFromHost();
}


void ExternalClient::reconnectTimeout() {
    connectToHost(m_address.host(),m_address.port());
}

void ExternalClient::slotconnected()
{
    m_alreadyWarnedNoHost = false;
    m_reconnect.stop();
    qDebug()<< m_plugin->base()->name() << "connected to"<<m_address.host()<<m_address.port();
}

void ExternalClient::slotdisconnected()
{
    qDebug()<< m_plugin->base()->name() << "disconnected from"<<m_address.host()<<m_address.port();
    disconnectFromHost();
    m_reconnect.start();
}

void ExternalClient::sloterror(QAbstractSocket::SocketError e)
{
    if (e == QAbstractSocket::HostNotFoundError) {
        if (m_alreadyWarnedNoHost) return;
        m_alreadyWarnedNoHost = true;
    }
    qWarning() << m_plugin->base()->name() << errorString();
}

void ExternalClient::slotreadyRead()
{
    if (!canReadLine()) {
        if (bytesAvailable()>1000) readAll();
        return;
    }

    while (bytesAvailable())
    {
        QByteArray line = readLine();
        line.chop(1);
        //QList<QByteArray> args = line.split(' ');
        qWarning()<< m_plugin->base()->name() << line;
    }
}

QList<AbstractStateTracker*> ExternalClient::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    return l;
}

void ExternalClient::hideVideo() {
    write("hideVideo\n");
}
void ExternalClient::closeFullscreen() {
    write("closeFullscreen\n");
}
void ExternalClient::setFilename(const QString& filename) {
    write("playvideo " + filename.toUtf8() +"\n");
}
void ExternalClient::setVolume(qreal newvol, bool relative) {
    if (relative)
        write("videovolume_relative " + QByteArray::number(newvol*100) + "\n");
    else {
        write("videovolume " + QByteArray::number(newvol*100) + "\n");
    }
}
void ExternalClient::setVolumeEvent(qreal newvol, bool relative) {
    if (relative)
        write("eventvolume_relative " + QByteArray::number(newvol*100) + "\n");
    else {
        write("eventvolume " + QByteArray::number(newvol*100) + "\n");
    }
}
void ExternalClient::stopvideo() {
    write("stopvideo\n");
}
void ExternalClient::stopevent() {
    write("stopevent\n");
}
void ExternalClient::setDisplay(int display) {
    write("display " + QByteArray::number(display) + "\n");
}
void ExternalClient::setClickActions(ActorAmbienceVideo::EnumOnClick leftclick, ActorAmbienceVideo::EnumOnClick rightclick, int restoretime) {
    write("clickactions " + QByteArray::number(leftclick) + " " + QByteArray::number(rightclick) + " " + QByteArray::number(restoretime) + "\n");
}
void ExternalClient::setDisplayState(int state) {
    write("displaystate " + QByteArray::number(state) + "\n");
}

void ExternalClient::showMessage(int duration, const QString& msg) {
    write("showmessage " + QByteArray::number(duration) + " " + msg.toUtf8() + "\n");
}

void ExternalClient::playEvent(const QString& filename) {
    write("playevent " + filename.toUtf8() +"\n");
}


