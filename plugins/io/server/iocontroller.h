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

class QUdpSocket;
class myPluginExecute;
class AbstractStateTracker;
class PinNameStateTracker;
class PinValueStateTracker;
class IOControllerThread;

class IOController : public QObject
{
    Q_OBJECT
public:
    IOController(myPluginExecute* plugin);
    ~IOController();
    QString getPinName ( const QString& pin );
    void setPin ( const QString& pin, bool value );
    void setPinName ( const QString& pin, const QString& name );
    void togglePin ( const QString& pin );
    bool getPin( const QString& pin ) const;
    int countPins();
    QList<AbstractStateTracker*> getStateTracker();
    void connectToIOs(int portSend, int portListen, const QString& user, const QString& pwd);
private:
    QString m_pluginname;
    QMap<QString, PinValueStateTracker*> m_values;
    QMap<QString, PinNameStateTracker*> m_names;
    QMap< QString, QPair<QHostAddress,uint> > m_mapPinToHost;
    int m_sendPort;
    QString m_user;
    QString m_pwd;
    QUdpSocket *m_listenSocket;
    QUdpSocket *m_writesocket;
private slots:
    void readyRead();
Q_SIGNALS:
    void stateChanged(AbstractStateTracker*);
    void dataLoadingComplete();
};
