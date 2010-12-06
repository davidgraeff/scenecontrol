#include "plugin_client.h"
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include <plugin.h>
#include "playlistsModel.h"

Q_EXPORT_PLUGIN2(libroomclient, myPluginClient)

myPluginClient::myPluginClient(QObject* parent) {
    Q_UNUSED(parent);
    m_base = new myPlugin();
	addModel(new PlaylistsModel());
}

myPluginClient::~myPluginClient() {
    //delete m_base;
    qDeleteAll(m_models);
}
