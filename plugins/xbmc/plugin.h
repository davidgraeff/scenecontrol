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
#include "shared/abstractplugin.h"
#include <QTcpSocket>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
public Q_SLOTS:
    // player
    void play();
    void pause();
    void stop();
    void next();
    void prev();
    void FastForward();
    void Rewind();
    // gui control
    void select();
    void down();
    void up();
    void left();
    void right();
    void home();
    void back();
    // volume
    void setVolume(int v);
private Q_SLOTS:
    void readyRead();
    void hostconnected();
    void hostdisconnected();
    void error(QAbstractSocket::SocketError);
private:
    QString m_host;
    int m_port;
    QTcpSocket m_socket;
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
};
