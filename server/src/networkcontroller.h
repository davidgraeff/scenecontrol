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
#include <QByteArray>
#include <QUuid>
#include <QStringList>
#include <QDBusConnection>

#define NETWORK_MIN_APIVERSION "2.0"
#define NETWORK_MAX_APIVERSION "2.0"
#define LISTENPORT 3101
#include <QDir>

struct ClientConnection
{
    QByteArray buffer;
    int bufferpos;
    int bufferBrakes;
    QTcpSocket* socket;
    ClientConnection(QTcpSocket* s) {bufferpos=0;bufferBrakes=0;socket=s;}
    ~ClientConnection() { delete socket;}
};

/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class NetworkController: public QTcpServer {
    Q_OBJECT
public:
    NetworkController();
    virtual ~NetworkController();
    bool start();
    void log(const char* msg);
    void backup_list_changed();
private:
    virtual void incomingConnection(int socketDescriptor);
    QMap<QTcpSocket*, ClientConnection*> connections;
    QByteArray getNextJson(ClientConnection*);
    void syncClient(QTcpSocket* socket);
    
private Q_SLOTS:
    void readyRead ();
    void disconnected();
public Q_SLOTS:
    void objectSync(QObject* p);
};

#endif /* TCPSERVER_H_ */
