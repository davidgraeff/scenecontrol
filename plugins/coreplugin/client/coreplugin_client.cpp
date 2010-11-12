#include "coreplugin_client.h"
#include <QDebug>
#include <QCoreApplication>

CorePluginClient::CorePluginClient(QObject* parent) {
    Q_UNUSED(parent);
    m_base = new CorePlugin();
}

CorePluginClient::~CorePluginClient() {
    delete m_base;
}
void CorePluginClient::stateChanged(AbstractStateTracker*) {}
void CorePluginClient::serviceRemoved(AbstractServiceProvider*) {}
void CorePluginClient::serviceChanged(AbstractServiceProvider*) {}
QList< ClientModel* > CorePluginClient::models() {
    return m_models;
}

