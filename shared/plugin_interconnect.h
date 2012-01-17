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
#include <QMap>
#include <QObject>
#include <QString>
#include <QLocalSocket>
#include <QLocalServer>
#include <QSet>
#include <QVariant>


#ifndef PLUGIN_ID
#error Define PLUGIN_ID before including this header!
#endif

class PluginInterconnect: public QLocalServer {
    Q_OBJECT
protected:
    PluginInterconnect();
public:
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QByteArray& data) = 0;
    bool sendCmdToPlugin(const QByteArray& plugin_id, const QByteArray& data);
    bool sendDataToPlugin(const QByteArray& plugin_id, const QVariant& data);
private Q_SLOT:
    void readyRead();
    void newConnection ();
private:
    QMap<QByteArray, QLocalSocket*> m_connectionsByID;
    QMap<QLocalSocket*, QByteArray> m_connectionsBySocket;
    QSet<QLocalSocket*> m_pendingConnections;
    QLocalSocket* getClientConnection(const QByteArray& plugin_id);
};

