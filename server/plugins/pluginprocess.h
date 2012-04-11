#pragma once

#include <QProcess>
#include <QLocalSocket>
#include <QVariantMap>
#include <QTimer>

class PluginController;

/**
 * Plugin processes need to be started by the server and are tracked via this PluginProcess class.
 * This is done this way to allow the server to terminate all started sub processes
 * if the server itself is going to shutdown and to ensure some security limitations.
 * 
 * Plugin process will be killed if they do not respond in an expected way on the input/output channel.
 * 
 * This object serves as a proxy to the abstractPlugin base class. Events, conditions and actions are
 * delivered via the callQtSlot method.
 */
class PluginProcess:  public QObject {
    Q_OBJECT
public:
	// Process related //
    PluginProcess(PluginController* controller, const QString& pluginid, const QString& instanceid);
    ~PluginProcess();
	void startProcess();
	/**
	 * Return true if the process responded as expected and is recogniced as valid plugin.
	 */
	bool isValid();
	/**
	 * Ask the plugin to terminate and wait for the process to finish
	 */
	void shutdown();
	QString getPluginid();
	QString getInstanceid();
	QLocalSocket* getSocket();
	void setSocket(QLocalSocket* socket);
private:
    PluginController* m_controller;
	QLocalSocket* m_pluginCommunication;
    qint64 m_pid;
    QString m_filename;
    QString m_pluginid;
	QString m_instanceid;
    QByteArray m_chunk;
	QTimer m_timeout;
	QMap<QByteArray, QVariantMap> m_configcache;
    bool writeToPlugin(const QVariantMap& data);
private Q_SLOTS:
    /** After a process got started it has 7 seconds to respond to the server via in/output channels
     * otherwise the process will get killed by the server and removed from the pending
     * processes of PluginController
	 */
    void responseTimeout();
	/**
	 * Something to read from the plugin
	 */
    void readyReadPluginData();
	/**
	 * State changed
	 */
    void communicationSocketStateChanged();
	void databaseStateChanged();
public Q_SLOTS:
    /// proxy methods: they send a request to the plugin process ////
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
    /// Responses are asynchron and propagated through the signal qtSlotResponse
    void callQtSlot(const QVariantMap& methodAndArguments, const QByteArray& responseid = QByteArray(), int sessionid = -1);
Q_SIGNALS:
    void qtSlotResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid);
};
