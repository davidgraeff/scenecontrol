#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QPluginLoader>
#include <QUuid>
#include <QDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include "paths.h"
#include <shared/pluginservicehelper.h>
#include "collectioncontroller.h"
#include "database.h"
#include "plugincontroller.h"
#include "socket.h"
#include "pluginprocess.h"
#include <shared/json.h>
#include <QElapsedTimer>

#define __FUNCTION__ __FUNCTION__
#define MAGICSTRING "roomcontrol_"
#define COMSERVERSTRING "server"

static PluginController* plugincontroller_instance = 0;

PluginController* PluginController::instance() {
    if (!plugincontroller_instance) {
        plugincontroller_instance = new PluginController();
    }
    return plugincontroller_instance;
}

PluginController::PluginController () {
    connect(&m_comserver, SIGNAL(newConnection()), SLOT(newConnection()));
}

PluginController::~PluginController()
{
    QMap<QString,PluginCommunication*>::iterator i = m_registeredevents.begin();
    for (;i!=m_registeredevents.end();++i) {
        PluginCommunication* executeplugin = i.value();
        executeplugin->unregister_event ( i.key() );
    }
    qDeleteAll(m_plugins);
    qDeleteAll(m_pluginprocesses);
}

void PluginController::newConnection()
{
    while ( QLocalSocket* socket = m_comserver.nextPendingConnection()) {
        QElapsedTimer t;
        t.start();
        while (!t.hasExpired(2000) && !socket->canReadLine()) {QCoreApplication::processEvents();}

        if (socket->canReadLine()) {
            const QByteArray plugin_id = socket->readLine();
            PluginCommunication* plugin =  m_pendingplugins.value(socket);
            if (!plugin)
                m_pendingplugins.insert( socket, new PluginCommunication( this, socket ) );
        } else {
            qWarning()<<"Socket tried to connect: No authentification" << socket->readAll();
            socket->deleteLater();
        }
    }
}

QMap< QString, PluginCommunication* >::iterator PluginController::getPluginIterator() {
    return m_plugins.begin();
}

PluginCommunication* PluginController::nextPlugin(QMap< QString, PluginCommunication* >::iterator& index) {
    if (m_plugins.end()==index) return 0;
    return (*(index++));
}

PluginCommunication* PluginController::getPlugin(const QString& pluginid) {
    return m_plugins.value(pluginid);
}

void PluginController::Event_add(const QString& id, const QVariantMap& event_data) {
    PluginCommunication* plugin = getPlugin ( ServiceData::pluginid ( event_data ) );
    if ( !plugin ) {
        qWarning() <<"Plugins: Cannot register event. No plugin found:"<<ServiceData::pluginid ( event_data ) << id;
        return;
    }

    qDebug() << "Plugins: register event:" << id << ServiceData::pluginid ( event_data ) << ServiceData::method ( event_data );
    plugin->unregister_event ( id );
    plugin->callQtSlot(event_data);
    m_registeredevents.insert(id, plugin);
}

void PluginController::Event_remove(const QString& id) {
    PluginCommunication* executeplugin = m_registeredevents.take ( id );
    if ( executeplugin ) {
        qDebug() << "Plugins: unregister event" << id << executeplugin;
        executeplugin->unregister_event ( id );
    }
}

void PluginController::settings(const QString& pluginid, const QString& key, const QVariantMap& data) {
    PluginCommunication* p = getPlugin(pluginid);
    if (!p) {
        qWarning() << "Plugins: Configuration for unknown plugin" << pluginid;
        return;
    } else {
        //qDebug() << "Plugins:" << data.size() << "Config" << key << "for plugin" << pluginid << "loaded";
    }
    p->configChanged(key.toAscii(), data);
}

bool PluginController::startplugins() {
    const QString name = QLatin1String(MAGICSTRING) + QLatin1String(COMSERVERSTRING);
    m_comserver.removeServer(name);
    if (!m_comserver.listen(name)) {
        return false;
    }
    const QDir plugindir = setup::pluginDir();
    QStringList pluginfiles = plugindir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    if (pluginfiles.empty())
        qWarning() << "Plugins: No plugins found in" << plugindir;

    for (int i=0;i<pluginfiles.size();++i) {
        // Start process by filename and insert into processes list
        m_pluginprocesses.insert( new PluginProcess( this, plugindir.absoluteFilePath ( pluginfiles[i] ) ) );
    }
    return true;
}

void PluginController::requestAllProperties(int sessionid) {
    {
        QMap<QString,PluginCommunication*>::iterator i = getPluginIterator();
        while (PluginCommunication* plugin = nextPlugin(i)) {
            plugin->requestProperties(sessionid);
        }
    }

    // Generate plugin list
    QStringList pluginlist;
    QMap<QString,PluginCommunication*>::iterator i = m_plugins.begin();
    for (;i!=m_plugins.end();++i) {
        pluginlist += (*i)->id;
    }
    ServiceData s = ServiceData::createNotification("plugins");
    s.setData("plugins", pluginlist);
    s.setPluginid("PluginController");
    Socket::instance()->propagateProperty(s.getData(), sessionid);
}

void PluginController::unloadPlugin(const QString& id) {
    PluginCommunication* p = m_plugins.take(id);
    if (!p)
        return;
    // Remove plugin process
    p->deleteLater();
    // Remove all registered events of this plugin out of the map m_registeredevents
    // (they are not active already due to removing the plugin process instance but to conserve memory)
    QMutableMapIterator<QString, PluginCommunication*> i = m_registeredevents;
    while (i.hasNext()) {
        i.next();
        if (i.value() == p)
            i.remove();
    }
}
void PluginController::removePluginFromPending(PluginCommunication* pluginprocess) {
    m_pendingplugins.remove(pluginprocess->getSocket());
    pluginprocess->deleteLater();
}

void PluginController::addPlugin(const QString& id, PluginCommunication* pluginprocess) {
    m_pendingplugins.remove(pluginprocess->getSocket());
    m_plugins.insert(id, pluginprocess);
}

void PluginController::removeProcess(PluginProcess* process) {
    m_pluginprocesses.remove(process);
    process->deleteLater();
}
