#include "plugin_client.h"
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include <plugin.h>
#include "pinsmodel.h"
#include <shared/client/modelstorage.h>

Q_EXPORT_PLUGIN2(libroomclient, myPluginClient)

myPluginClient::myPluginClient(QObject* parent) : ClientPlugin() {
    Q_UNUSED(parent);
    m_base = new myPlugin();
}

myPluginClient::~myPluginClient() {
    //delete m_base;
}

void myPluginClient::init() {
    ModelStorage* modelstorage = ModelStorage::instance();
    modelstorage->registerClientModel(new PinsModel());
}