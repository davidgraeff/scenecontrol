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

class AbstractStateTracker;
class AuthThread;
class AbstractServiceProvider;
class ServiceController;
struct ClientConnection
{
    QByteArray buffer;
    int bufferpos;
    int bufferBrakes;
	QSslSocket* socket;
	bool auth;
	ClientConnection(QSslSocket* s) {bufferpos=0;bufferBrakes=0;socket=s;auth=false;}
    ~ClientConnection() { delete socket;}
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
	QMap<QTcpSocket*, ClientConnection*> m_connections;
    QByteArray getNextJson(ClientConnection*);
	void syncClient(QSslSocket* socket);
    
private Q_SLOTS:
    void readyRead ();
    void disconnected();
	void auth_failed(QObject* ptr);
	void auth_success(QObject* ptr);
public Q_SLOTS:
	void serviceSync(AbstractServiceProvider* p);
	void statetrackerSync(AbstractStateTracker* p);
};

#endif /* TCPSERVER_H_ */
