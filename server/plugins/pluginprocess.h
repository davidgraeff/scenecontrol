#pragma once

#include <QProcess>
#include <QLocalSocket>
#include <QVariantMap>

class PluginController;

/**
 * Every plugin process that is started via the server (and not running before
 * or started externally) is tracked via this PluginProcess class.
 * This is mostly done to allow the server to terminate all started sub processes
 * if the server itself is going to shutdown.
 */
class PluginProcess:  public QObject {
    Q_OBJECT
public:
    PluginProcess(PluginController* controller, const QString& filename);
    ~PluginProcess();
	void finishProcess();
private:
    PluginController* m_controller;
    QProcess m_pluginProcess;
    QString m_filename;
    bool m_aboutToFree;
private Q_SLOTS:
    /** After a process got started it has 7 seconds to establish a communication socket
     * otherwise the process will get killed by the server and removed from the pending
     * processes of PluginController
	 */
    void startTimeout();
public slots:
    void finished();
};
