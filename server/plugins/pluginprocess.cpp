#include "pluginprocess.h"
#include "plugincontroller.h"
#include <QTimer>
#include "paths.h"
#include "config.h"

PluginProcess::PluginProcess(PluginController* controller, const QString& filename)
        : m_controller(controller), m_filename(filename), m_aboutToFree(false) {
    m_pluginProcess.setProcessChannelMode(QProcess::ForwardedChannels);
	qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    connect(&m_pluginProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(finished()));
    m_pluginProcess.start(filename);
    if (m_pluginProcess.waitForStarted())
        qDebug() << "Started plugin process" << filename;
    else
        qWarning() << "Failed starting plugin process" << filename;
    // Start timer for killing process if it does not establish a communication line
    //QTimer::singleShot(3000, this, SLOT(startTimeout()));
}

PluginProcess::~PluginProcess() {
    m_aboutToFree = true;
    if (m_pluginProcess.state()==QProcess::Running) {
        m_pluginProcess.terminate();
    }
    m_pluginProcess.waitForFinished();
}

void PluginProcess::finished() {
	int exitCode = m_pluginProcess.exitCode();
	QProcess::ExitStatus exitStatus = m_pluginProcess.exitStatus();
    if (exitStatus==QProcess::CrashExit && exitCode != 0)
        qWarning() << "Server: Plugin crashed" << m_filename << m_pluginProcess.pid();
    else
        qDebug() << "Server: Plugin finished" << m_filename << m_pluginProcess.pid();
    if (!m_aboutToFree)
	this->deleteLater();
    m_controller->removeProcess(this);
}

void PluginProcess::startTimeout() {
    qWarning() << "Server: Plugin process timeout" << m_filename << m_pluginProcess.pid();
    if (m_pluginProcess.state()==QProcess::Running) {
        m_pluginProcess.kill();
        m_pluginProcess.waitForFinished();
    }
    m_controller->removeProcess(this);
}
