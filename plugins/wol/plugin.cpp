#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "services/actorwol.h"
#include "services_server/actorwolServer.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
  m_base = new myPlugin();
}

myPluginExecute::~myPluginExecute() {
  //delete m_base;
}


void ServiceWOLExecute::execute() {
    ActorWOL* s = service<ActorWOL>();
    Q_ASSERT(s);
    QStringList parts = s->mac().split(QLatin1Char(':'));
    if (parts.size()!=6) return;
    QByteArray mac;
    for (int i=0;i<6;++i)
        mac.append(QByteArray::fromHex(parts[i].toAscii()));

    // 6 mal FF
    const char header[] = {255,255,255,255,255,255};
    QByteArray bytes(header);
    // 16 mal mac
    for (int i=0;i<16;++i)
        bytes.append(mac);

    QUdpSocket socket;
    socket.writeDatagram(bytes,QHostAddress::Broadcast,9);
}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorWOL::staticMetaObject.className()) {
        return new ServiceWOLExecute((ActorWOL*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    return a;
}
