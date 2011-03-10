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

void ActorAmbienceCmdServer::execute()
{
    ActorAmbienceCmd* base = service<ActorAmbienceCmd>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        switch (base->cmd()) {
        case ActorAmbienceCmd::CloseFullscreen:
            client->closeFullscreen();
            break;
        case ActorAmbienceCmd::ScreenOff:
            client->setDisplayState(0);
            break;
        case ActorAmbienceCmd::ScreenOn:
            client->setDisplayState(1);
            break;
        case ActorAmbienceCmd::ScreenToggle:
            client->setDisplayState(2);
            break;
        case ActorAmbienceCmd::HideVideo:
            client->hideVideo();
            break;
        case ActorAmbienceCmd::StopVideo:
            client->stopvideo();
            break;
        default:
            break;
        }
    }
}

void ActorAmbienceVideoServer::execute()
{
    ActorAmbienceVideo* base = service<ActorAmbienceVideo>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        client->setDisplay(base->display());
        client->setClickActions(base->onleftclick(),base->onrightclick(), base->restoretime());
        client->setVolume(base->volume());
        client->setFilename(base->filename());
    }
}

void ActorAmbienceVolumeServer::execute()
{
    ActorAmbienceVolume* base = service<ActorAmbienceVolume>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        client->setVolume(base->volume(),base->relative());
    }
}

void ActorEventServer::execute()
{
    ActorEvent* base = service<ActorEvent>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        if (base->filename().size())
            client->playEvent(base->filename());
        if (base->title().size())
            client->showMessage(base->duration(),base->title());
    }
}

void ActorEventVolumeServer::execute()
{
    ActorEventVolume* base = service<ActorEventVolume>();
    QList< ExternalClient* > clients = m_plugin->specificClients(base->host());
    foreach (ExternalClient* client, clients) {
        client->setVolume(base->volume(),base->relative());
    }
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
