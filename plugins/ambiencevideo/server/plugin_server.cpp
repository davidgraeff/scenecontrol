#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "externalclient.h"
#include <services/actorambiencevideo.h>
#include <services/actorambiencecmd.h>
#include <services/actorambiencevolume.h>
#include <services_server/actorambiencevideoServer.h>
#include <services_server/actorambiencecmdServer.h>
#include <services_server/actorambiencevolumeServer.h>
#include "statetracker/eventST.h"
#include "statetracker/eventvolumeST.h"
#include "services/actorevent.h"
#include "services/actoreventvolume.h"
#include "services_server/actoreventServer.h"
#include "services_server/actoreventvolumeServer.h"
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
    m_base = new myPlugin();
    _config(this);
}

myPluginExecute::~myPluginExecute() {
        qDeleteAll(m_clients);
        m_clients.clear();
    //delete m_base;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorAmbienceVideo::staticMetaObject.className())
        return new ActorAmbienceVideoServer((ActorAmbienceVideo*)service, this);
    else if (idb == ActorAmbienceCmd::staticMetaObject.className())
        return new ActorAmbienceCmdServer((ActorAmbienceCmd*)service, this);
    else if (idb == ActorAmbienceVolume::staticMetaObject.className())
        return new ActorAmbienceVolumeServer((ActorAmbienceVolume*)service, this);
    else if (idb == ActorEvent::staticMetaObject.className()) {
        return new ActorEventServer((ActorEvent*)service, this);
    } else if (idb == ActorEventVolume::staticMetaObject.className()) {
        return new ActorEventVolumeServer((ActorEventVolume*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> list;
    foreach(ExternalClient* client, m_clients) {
        list.append(client->getStateTracker());
    }
    return list;
}

void myPluginExecute::setSetting(const QString& name, const QVariant& value) {
    ExecutePlugin::setSetting(name, value);
    if (name == QLatin1String("servers")) {
        qDeleteAll(m_clients);
        m_clients.clear();
        QStringList strings = value.toString().split(QLatin1Char(';'));
        foreach(QString address, strings) {
            const QStringList data(address.split(QLatin1Char(':')));
            if (data.size()!=2) continue;
            m_clients.append(new ExternalClient(this,data[0], data[1].toInt()));
        }
    }
}

QList< ExternalClient* > myPluginExecute::specificClients(const QString& host) {
    if (host.isEmpty()) return m_clients;
    QList<ExternalClient*> r;
    foreach(ExternalClient* client, m_clients) {
        if (client->peerAddress() == QHostAddress(host)) r.append(client);
    }
    return r;
}
