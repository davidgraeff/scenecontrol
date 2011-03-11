/*
 * Network Controller
 *
 *  Created on: 10.02.2010
 *      Author: David Gräff
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
