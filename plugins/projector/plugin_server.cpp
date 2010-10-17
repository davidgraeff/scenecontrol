#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "../server/executeservice.h"
#include "plugin.h"
#include "statetracker/projectorstatetracker.h"
#include "services/actorprojector.h"
#include "services_server/actorprojectorServer.h"
#include <qfile.h>

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

#define DEVICE "/dev/ttyS0"
myPluginExecute::myPluginExecute() : ExecutePlugin(),
        m_serial(QLatin1String(DEVICE),QextSerialPort::EventDriven) {
    m_base = new myPlugin();
    m_ProjectorStateTracker = new ProjectorStateTracker();
    m_serial.setBaudRate(BAUD19200);
    m_serial.setFlowControl(FLOW_OFF);
    m_serial.setParity(PAR_NONE);
    m_serial.setDataBits(DATA_8);
    m_serial.setStopBits(STOP_1);
    connect(&m_serial, SIGNAL(readyRead()), SLOT(readyRead()));
    buffer[3] = '\r';
    if (!QFile::exists(QLatin1String(DEVICE))) {
        qWarning() << "Projector: device not found"<<DEVICE;
        return;
    }
    if (!m_serial.open(QIODevice::ReadWrite)) {
        qWarning() << "Projector: rs232 init fehler";
    }
}

myPluginExecute::~myPluginExecute() {
    //delete m_base;
    delete m_ProjectorStateTracker;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorProjector::staticMetaObject.className()) {
        return new ActorProjectorServer((ActorProjector*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    m_ProjectorStateTracker->setState(0);
    a.append(m_ProjectorStateTracker);
    return a;
}

void myPluginExecute::setCommand(ActorProjector::ProjectorControl c) {
    switch (c) {
    case ActorProjector::ProjectorOn:
        strncpy(buffer, "C00", 3);
        break;
    case ActorProjector::ProjectorOff:
        strncpy(buffer, "C01", 3);
        break;
    case ActorProjector::ProjectorVideoMute:
        strncpy(buffer, "C0D", 3);
        break;
    case ActorProjector::ProjectorVideoUnMute:
        strncpy(buffer, "C0E", 3);
        break;
    case ActorProjector::ProjectorLampNormal:
        strncpy(buffer, "C74", 3);
        break;
    case ActorProjector::ProjectorLampEco:
        strncpy(buffer, "C75", 3);
        break;
    default:
        return;
    };

    if (!m_serial.write(buffer,4)) {
        qWarning() << "projector send failed\n";
        return;
    }

    m_ProjectorStateTracker->setState(c);
    emit stateChanged(m_ProjectorStateTracker);
}

void myPluginExecute::readyRead() {
    QByteArray bytes;
    int a = m_serial.bytesAvailable();
    bytes.resize(a);
    m_serial.read(bytes.data(), bytes.size());
}
