/*
 * Network Controller
 *
 *  Created on: 10.02.2010
 *      Author: David Gräff
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QUuid>
#include <QStringList>
#include <QDBusConnection>

#define NETWORK_MIN_APIVERSION "3.0"
#define NETWORK_MAX_APIVERSION "3.0"
#define LISTENPORT 3101
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
    QString user() {
        return m_user;
    }
    void setAuth(const QString& user) {
        m_user = user;
        m_auth = true;
        m_authTimer.stop();
    }
    bool auth() {
        return m_auth;
    }
    ClientConnection(QSslSocket* s) {
        bufferpos=0;
        bufferBrakes=0;
        socket=s;
        m_auth=false;
        connect(&m_authTimer,SIGNAL(timeout()),SLOT(timeout()));
        m_authTimer.start(1000*60*2);
    }
    ~ClientConnection() {
        delete socket;
    }
private Q_SLOTS:
    void timeout() {
        emit timeoutAuth(socket);
    }
Q_SIGNALS:
    void timeoutAuth(QSslSocket* socket);
};

/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class NetworkController: public QTcpServer {
    Q_OBJECT
public:
    NetworkController(QDBusConnection dbusconnection);
    virtual ~NetworkController();
    void setServiceController(ServiceController* controller) ;
    bool start();
    void log(const char* msg);

    // DBUS
    QDBusConnection* dbus();

private:
    QDBusConnection m_dbusconnection;
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
public Q_SLOTS:
    void serviceSync(AbstractServiceProvider* p);
    void statetrackerSync(AbstractStateTracker* p);
};

#endif /* TCPSERVER_H_ */
