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
#include <shared/abstractplugin.h>

ExternalClient::ExternalClient(AbstractPlugin* plugin, const QString& host, int port) : m_plugin(plugin), m_connected(false)
{
    connect(&m_reconnect,SIGNAL(timeout()),SLOT(reconnectTimeout()));
    m_reconnect.setInterval(60000);

    connect(this, SIGNAL(connected()), SLOT(slotconnected()));
    connect(this, SIGNAL(disconnected()), SLOT(slotdisconnected()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(sloterror(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(readyRead()), SLOT(slotreadyRead()));

    m_host = host;
    m_port = port;

    reconnectTimeout();
    m_reconnect.start();
}

ExternalClient::~ExternalClient()
{
    disconnectFromHost();
}


void ExternalClient::reconnectTimeout() {
    if (state() == UnconnectedState)
        connectToHost(m_host,m_port);
}

void ExternalClient::slotconnected()
{
    m_reconnect.stop();
    m_connected = true;
	emit stateChanged(this);
}

void ExternalClient::slotdisconnected()
{
    m_connected = false;
	emit stateChanged(this);
    disconnectFromHost();
    m_reconnect.start();
}

void ExternalClient::sloterror(QAbstractSocket::SocketError e) {Q_UNUSED(e);}

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
        qWarning()<< m_plugin->pluginid() << line;
    }
}

void ExternalClient::showVideo(const QString& filename, int display) {
    write("playvideo;" + filename.toUtf8() + ";" + QByteArray::number(display) +"\n");
}

void ExternalClient::setSystemVolume(qreal newvol, bool relative) {
    if (relative)
        write("volume_relative;" + QByteArray::number(newvol*100) + "\n");
    else {
        write("volume;" + QByteArray::number(newvol*100) + "\n");
    }
}

void ExternalClient::setDisplayState(int state, int display) {
    write("displaystate;" + QByteArray::number(state) + ";" + QByteArray::number(display) + "\n");
}

void ExternalClient::showMessage(int duration, const QString& msg, const QString& audiofile) {
    write("showmessage;" + QByteArray::number(duration) + ";" + msg.toUtf8() + ";" + audiofile.toUtf8() + "\n");
}

