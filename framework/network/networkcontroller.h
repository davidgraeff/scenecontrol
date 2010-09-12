/*
 * Network Controller
 *
 *  Created on: 10.02.2010
 *      Author: David Gräff
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_
#include <QTcpSocket>
#include <QByteArray>
#include <QUuid>
#include <QStringList>
#include <QDBusConnection>
#include <QTimer>
#include "stateTracker/programstatetracker.h"

#define NETWORK_MIN_APIVERSION "2.0"
#define NETWORK_MAX_APIVERSION "2.0"
#define LISTENPORT 3101

/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class NetworkController: public QTcpSocket {
    Q_OBJECT
public:
    NetworkController();
    virtual ~NetworkController();
    void start(const QString& ip, int port);
    void start();
    void resync();
    void refresh();
    void restart();
	void backup();
    void backup_list();
	void backup_restore(const QString& id);
	void backup_remove(const QString& id);
    QString serverversion();

private:
    QByteArray getNextJson();
    QByteArray m_buffer;
    int m_bufferpos;
    int m_bufferBrakes;
    ProgramStateTracker m_serverstate;
    QTimer serverTimeout;
    int m_port;
    QString m_ip;

private Q_SLOTS:
    void slotreadyRead ();
    void timeout();
    void slotconnected();
    void slotdisconnected();
    void sloterror(QAbstractSocket::SocketError);
public Q_SLOTS:
    void objectSync(QObject* p);
Q_SIGNALS:
    void connectedToValidServer();
	void logmsg(const QString& log);
	void backupPaths(const QStringList&);
};

#endif /* TCPSERVER_H_ */
