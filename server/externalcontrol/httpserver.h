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

	Purpose: https server
*/

#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QUuid>
#include <QStringList>

#include <QDir>
#include <QTimer>

class AbstractStateTracker;
class AuthThread;
class AbstractServiceProvider;
class ServiceController;

class ClientConnection : public QObject
{
    Q_OBJECT
private:
    bool m_auth;
    QTimer m_authTimer;
    QString m_user;
public:
    QByteArray buffer;
    int bufferpos;
    int bufferBrakes;
    QSslSocket* socket;
    QString user() ;
    void setAuth(const QString& user) ;
    bool auth() ;
    ClientConnection(QSslSocket* s) ;
    ~ClientConnection() ;
private Q_SLOTS:
    void timeout() ;
Q_SIGNALS:
    void timeoutAuth(QSslSocket* socket);
};

/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class NetworkController: public QTcpServer {
    Q_OBJECT
public:
    NetworkController();
    virtual ~NetworkController();
    void setServiceController(ServiceController* controller) ;
    bool start();
    void log(const char* msg);

private:
    ServiceController* m_service;
    AuthThread* m_auththread;
    virtual void incomingConnection(int socketDescriptor);
    QMap<QSslSocket*, ClientConnection*> m_connections;
    QByteArray getNextJson(ClientConnection*);
    void syncClient(QSslSocket* socket);

private Q_SLOTS:
    void readyRead ();
    void disconnected();
    void auth_success(QObject* socketptr, const QString& name);
    void auth_failed(QObject* socketptr, const QString& name);
    void timeoutAuth(QSslSocket* socket);
    void peerVerifyError(QSslError);
    void sslErrors(QList<QSslError>);
public Q_SLOTS:
    void serviceSync(AbstractServiceProvider* p);
    void statetrackerSync(AbstractStateTracker* p);
};
