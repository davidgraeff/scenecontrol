#include "coreplugin_client.h"
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "backupsmodel.h"
#include <shared/client/modelstorage.h>

Q_EXPORT_PLUGIN2(libroomclient, CorePluginClient)

CorePluginClient::CorePluginClient(QObject* parent) {
    Q_UNUSED(parent);
    m_base = new myPlugin();
}

CorePluginClient::~CorePluginClient() {
    //delete m_base;
}

void CorePluginClient::init() {
    ModelStorage* modelstorage = ModelStorage::instance();
    modelstorage->registerClientModel(new BackupsModel());
}
