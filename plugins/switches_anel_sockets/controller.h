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
#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QPair>
#include <QUdpSocket>

class AbstractPlugin;
class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(AbstractPlugin* plugin);
    ~Controller();
    void reinitialize();
    QString getChannelName ( const QString& pin );
    void setChannel ( const QString& pin, bool value );
    void setChannelName ( const QString& pin, const QString& name );
    void toggleChannel ( const QString& pin );
    bool getChannel( const QString& pin ) const;
    int countChannels();
    void connectToIOs(int portSend, int portListen, const QString& user, const QString& pwd);

    struct iochannel {
        int value;
        QString name;
        iochannel() {
            value = -1;
        }
    };
    QMap<QString,iochannel> m_ios;
private:
    AbstractPlugin* m_plugin;
    QMap< QString, QPair<QHostAddress,uint> > m_mapChannelToHost;
    int m_sendPort;
    QString m_user;
    QString m_pwd;
    QUdpSocket *m_listenSocket;
    QUdpSocket *m_writesocket;
    QTimer m_cacheTimer;
    QMap<QString, unsigned char> m_cache;
private slots:
    void readyRead();
    void cacheToDevice();
Q_SIGNALS:
    void dataChanged(const QString& id, const QString& name, int value);
    void dataLoadingComplete();
};
