#pragma once

#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QVariantMap>
#include <QTimer>

class PluginController;

class PluginProcess:  public QObject {
    Q_OBJECT
public:
    PluginProcess(PluginController* controller, const QString& filename);
    ~PluginProcess();
private:
    PluginController* m_controller;
    QProcess m_pluginProcess;
    QString m_filename;
    bool m_aboutToFree;
private Q_SLOTS:
    // After a process got started it has 3 seconds to establish a communication socket
    // otherwise the process will get killed by the server and removed from the pending
    // processes of PluginController
    void startTimeout();
public slots:
    void finished(int);
};

class PluginCommunication: public QObject {
    Q_OBJECT
public:

    PluginCommunication(PluginController* controller, QLocalSocket* socket);
    ~PluginCommunication();
    QLocalSocket* getSocket();
    QString id;
    void setVersion(const QString& version) ;
    //// proxy methods: they send a request to the plugin process ////
    /// Proxy Method: Initialize plugin
    void initialize();
    /// Proxy Method: Clear all ressources
    void clear();
    /// Proxy Method: A configuration (key,value) touple changed
    void configChanged(const QByteArray& configid, const QVariantMap& data);
    /// Proxy Method: Request plugin properties. They are send from plugin to the server
    /// via the changeProperty method. This special request can be identified by the sessionid.
    void requestProperties(int sessionid);
    /// Proxy Method: Unregister event
    void unregister_event ( const QString& eventid );
    /// Proxy Method: Session started or finished
    void session_change ( int sessionid, bool running );
    /// Proxy Method: Call Qt Slot of the plugin. The QVariantMap have to contain at least a method_ member
    bool callQtSlot(const QVariantMap& methodAndArguments, QVariant* returnValue = 0);
private:
    PluginController* m_controller;
    QLocalSocket* m_pluginCommunication;
    QByteArray m_chunk;
    QString m_version;
    bool writeToPlugin(const QVariantMap& data);
private Q_SLOTS:
    void readyRead();
    void stateChanged(QLocalSocket::LocalSocketState state);
    // After a process got started it has 3 seconds to establish a communication socket
    // otherwise the process will get killed by the server and removed from the pending
    // processes of PluginController
    void startTimeout();
};
