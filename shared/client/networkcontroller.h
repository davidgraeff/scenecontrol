/*
 * Network Controller
 *
 *  Created on: 10.02.2010
 *      Author: David Gräff
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QUuid>
#include <QStringList>
#include <QDBusConnection>
#include <QTimer>

class ClientModel;
class AbstractStateTracker;
class ClientPlugin;
class AbstractServiceProvider;
/**
 * Manages incoming connections and delegate commands to the RoomControlServer
 */
class NetworkController: public QSslSocket {
    Q_OBJECT
public:
    NetworkController();
    virtual ~NetworkController();
    void start(const QString& ip, int port);
    void authenticate(const QString& user, const QString& pwd);
    void start();
    QString serverversion();
    QStringList supportedPlugins();
    ClientModel* model(const QString& id) ;
    QMap<QString, ClientModel*> models();
    void registerClientModel(ClientModel* model) ;
    QMap<QString, ClientPlugin*> plugin_providers();
    void loadPlugins();
private:
    QByteArray getNextJson();
    QByteArray m_buffer;
    int m_bufferpos;
    int m_bufferBrakes;
    QTimer serverTimeout;
    QString m_serverversion;
    QStringList m_supportedPlugins;
    int m_port;
    QString m_ip;
    QString m_user;
    QString m_pwd;
    enum connection_state {
        NotConnectedState,
        WaitingForServerState,
        AuthentificatingState,
        TransferingState,
        ConnectedState
    } networkstate;
    // service providers and stateTracker
    QList<AbstractServiceProvider*> m_servicesList;
    QMap<QString, AbstractServiceProvider*> m_services;

    // plugins
    QList<ClientPlugin*> m_plugins;
    QMap<QString, ClientPlugin*> m_plugin_provider;
    QMap<QString, ClientModel*> m_models;
    bool generate(const QVariantMap& data);

private Q_SLOTS:
    void slotreadyRead ();
    void timeout();
    void slotconnected();
    void slotdisconnected();
    void sloterror(QAbstractSocket::SocketError);
public Q_SLOTS:
    void serviceSync(AbstractServiceProvider* p);
    void executeService(AbstractServiceProvider*);
Q_SIGNALS:
    void connectedToValidServer();
    // failure messages
    void server_versionmissmatch(const QString& serverversion);
    void auth_failed();
    void auth_notaccepted();
    void auth_success();
    // server informations
    void timeoutData();
    void syncComplete();
    void logmsg(const QString& log);
    void auth_required(int timeout_ms);
    //plugins
    void stateTrackerChanged(AbstractStateTracker*);
    void serviceRemoved(AbstractServiceProvider*);
    void serviceChanged(AbstractServiceProvider*);
    void clearPlugins();
};

#endif /* TCPSERVER_H_ */
