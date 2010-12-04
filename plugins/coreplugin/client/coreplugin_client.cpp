#include "coreplugin_client.h"
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "backupsmodel.h"

Q_EXPORT_PLUGIN2(libroomclient, CorePluginClient)

CorePluginClient::CorePluginClient(QObject* parent) {
    Q_UNUSED(parent);
    m_base = new myPlugin();
    addModel(new BackupsModel());
}

CorePluginClient::~CorePluginClient() {
    //delete m_base;
	qDeleteAll(m_models);
}
