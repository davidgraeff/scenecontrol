#include "plugin_client.h"
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include <plugin.h>

Q_EXPORT_PLUGIN2(libroomclient, myPluginClient)

myPluginClient::myPluginClient(QObject* parent) {
    Q_UNUSED(parent);
    m_base = new myPlugin();
}

myPluginClient::~myPluginClient() {
    delete m_base;
    qDeleteAll(m_models);
}
void myPluginClient::stateChanged(AbstractStateTracker*) {}
void myPluginClient::serviceRemoved(AbstractServiceProvider*) {}
void myPluginClient::serviceChanged(AbstractServiceProvider*) {}
QList< ClientModel* > myPluginClient::models() {
    return m_models;
}
void myPluginClient::handle_ActorPlaylistTrack_playlistid_indexchanged(QModelIndex) {
  //temp track model: reload with new playlist
}

