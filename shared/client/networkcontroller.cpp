/*
 * NetworkController.cpp
 *
 *  Created on: 19.09.2009
 *      Author: david
 */

#include "networkcontroller.h"
// QT
#include <QHostAddress>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <QLibrary>
#include <QDebug>

#include <solid/networking.h>
#include "shared/qjson/qobjecthelper.h"
#include "shared/qjson/parser.h"
#include "shared/qjson/serializer.h"
#include "shared/abstractserviceprovider.h"
#include "shared/client/clientplugin.h"
#include <shared/abstractplugin.h>

#include "config.h"
#include <shared/categorize/category.h>
#include <shared/categorize/profile.h>
#include "servicestorage.h"
#include "modelstorage.h"

#define __FUNCTION__ __FUNCTION__

NetworkController* NetworkController::m_instance = 0;

NetworkController* NetworkController::instance() {
    if (!m_instance) m_instance=new NetworkController();
    return m_instance;
}

NetworkController::NetworkController()
{
    // init storages
    m_servicestorage = ServiceStorage::instance(true);
    connect(m_servicestorage,SIGNAL(serviceExecute(AbstractServiceProvider*)),SLOT(executeService(AbstractServiceProvider*)));
    connect(m_servicestorage,SIGNAL(serviceSync(AbstractServiceProvider*)),SLOT(serviceSync(AbstractServiceProvider*)));
    m_modelstorage = ModelStorage::instance(true);

    // init network
    networkstate =  NotConnectedState;
    m_bufferpos=0;
    m_bufferBrakes=0;
    connect(&serverTimeout,SIGNAL(timeout()),SLOT(timeout()));
    serverTimeout.setSingleShot(true);
    serverTimeout.setInterval(5000);
    setPeerVerifyMode(QSslSocket::VerifyNone);

    connect ( this, SIGNAL ( readyRead() ), SLOT ( slotreadyRead() ) );
    connect ( this, SIGNAL ( connected() ), SLOT( slotconnected()) );
    connect ( this, SIGNAL ( disconnected() ), SLOT( slotdisconnected()) );
    connect ( this, SIGNAL ( error(QAbstractSocket::SocketError) ), SLOT( sloterror(QAbstractSocket::SocketError) ));

    connect( Solid::Networking::notifier(), SIGNAL(shouldDisconnect()), SLOT(timeout()) );
}

NetworkController::~NetworkController()
{
    disconnectFromHost();
    slotdisconnected();
    foreach(ClientPlugin* p, m_plugins) {
        qDebug() << "Unload plugin" << p->base()->name();
        delete p;
    }
    delete m_servicestorage;
    delete m_modelstorage;
    m_plugin_provider.clear();
    m_plugins.clear();
}

void NetworkController::slotconnected()
{
    qDebug() << "Connected to"<<peerAddress().toString()<<peerPort();
    if (m_plugins.empty()) loadPlugins();
    networkstate =  WaitingForServerState;
    serverTimeout.start();
}

void NetworkController::slotdisconnected()
{
    if (state() != UnconnectedState)
        disconnectFromHost();
    serverTimeout.stop();
    networkstate =  NotConnectedState;
    // First clear plugins and models and then delete services
    m_servicestorage->clear();
    m_buffer.clear();
}

void NetworkController::timeout()
{
    if (networkstate==WaitingForServerState)
        emit timeoutData();
    disconnectFromHost();
}

void NetworkController::sloterror(QAbstractSocket::SocketError e)
{
    switch (e) {
    default:
        qWarning()<<"Error in SSL socket: "<<e;
        break;
    case SslHandshakeFailedError:
        qWarning()<<"SSL Handshake failed";
        break;
    }
}


void NetworkController::start(const QString& ip, int port)
{
    disconnectFromHost();
    networkstate =  NotConnectedState;
    m_ip = ip;
    m_port = port;
    connectToHostEncrypted ( ip, port );
}

void NetworkController::start()
{
    start(m_ip,m_port);
}

QByteArray NetworkController::getNextJson()
{
    for (; m_bufferpos<m_buffer.size(); ++m_bufferpos)
    {
        if (m_buffer[m_bufferpos] == '{')
            ++m_bufferBrakes;
        else if (m_buffer[m_bufferpos] == '}')
            --m_bufferBrakes;

        if (m_bufferpos>1 && m_bufferBrakes == 0)
        {
            const QByteArray res = m_buffer.mid(0,m_bufferpos+1);
            m_buffer.remove(0,m_bufferpos+1);
            m_bufferpos = 0;
            return res;
        }
    }

    // no paragraph {..} found
    return QByteArray();
}

void NetworkController::serviceSync(AbstractServiceProvider* p)
{
    // do nothing if not connected
    if (!isWritable()) return;

    QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
    if (p->dynamicPropertyNames().contains("remove"))
        variant.insert(QLatin1String("remove"), true);
    if (p->dynamicPropertyNames().contains("iexecute"))
        variant.insert(QLatin1String("iexecute"), true);

    QJson::Serializer serializer;
    QByteArray cmdbytes = serializer.serialize(variant);
    //qDebug() << "sync" << cmdbytes;
    write ( cmdbytes );
}

void NetworkController::executeService(AbstractServiceProvider* p)
{
    // do nothing if not connected
    if (!isWritable()) return;

    QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
    variant.insert(QLatin1String("iexecute"), true);
    delete p;

    QJson::Serializer serializer;
    QByteArray cmdbytes = serializer.serialize(variant);

    write ( cmdbytes );
}

void NetworkController::slotreadyRead()
{
    if ( this->bytesAvailable() )
    {
        m_buffer += this->readAll();
        // if buffer>2MByte discard
        if ( m_buffer.size() >1024*2000 )
        {
            m_buffer.clear();
            m_bufferBrakes = 0;
            m_bufferpos = 0;
            qWarning() << "receive incomplete" << m_buffer;
            return;
        }
    } else return;


    QByteArray cmdstring;
    while (1) {
        cmdstring = getNextJson();
        if (cmdstring.isEmpty()) return;

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse (cmdstring, &ok).toMap();
        const QByteArray type = result.value(QLatin1String("type")).toByteArray();
        if (!ok || !result.contains(QLatin1String("type"))) {
            qWarning() <<  "Failure: Parsing" << cmdstring;
        } else if (networkstate==WaitingForServerState && type=="serverinfo") {
            serverTimeout.stop();
            m_serverversion = result.value(QLatin1String("version")).toString();
            if (m_serverversion.toAscii()<ROOM_NETWORK_MIN_APIVERSION &&
                    m_serverversion.toAscii()>ROOM_NETWORK_MAX_APIVERSION)
            {
                disconnectFromHost();
                emit server_versionmissmatch(m_serverversion);
                m_buffer.clear();
                break;
            } else {
                emit connectedToValidServer();
                m_supportedPlugins = result.value(QLatin1String("plugins")).toString().split(QLatin1Char(','));
                networkstate =  (result.value(QLatin1String("auth"))==QLatin1String("required")) ? AuthentificatingState : TransferingState;
                if (networkstate==AuthentificatingState) emit auth_required(result.value(QLatin1String("auth_timeout")).toInt());
            }
        } else if (networkstate==AuthentificatingState && type=="auth") {
            QByteArray status = result.value(QLatin1String("result")).toByteArray();
            if (status=="ok")  {
                networkstate=TransferingState;
                emit auth_success();
            } else if (status=="failed")  {
                disconnectFromHost();
                emit auth_failed();
            } else if (status=="missing")  {
                qDebug() << "Auth is missing unexspectedly";
                disconnectFromHost();
                emit auth_failed();
            } else if (status=="notaccepted")  {
                disconnectFromHost();
                emit auth_notaccepted();
            } else if (status=="timeout")  {
                disconnectFromHost();
                emit auth_failed();
            }
        } else if (networkstate==TransferingState && type=="complete") {
            networkstate =  ConnectedState;
            emit syncComplete();
        } else if (result.value(QLatin1String("type"))==QLatin1String("log")) {
            emit logmsg(result.value(QLatin1String("data")).toString());
        } else if (networkstate==TransferingState || networkstate == ConnectedState) {
            generate(result);
        }
    };
}

QString NetworkController::serverversion()
{
    return m_serverversion;
}

void NetworkController::authenticate(const QString& user, const QString& pwd) {
    if (networkstate==AuthentificatingState) {
        QByteArray data("{\"type\" : \"auth\", \"name\" : \""+user.toUtf8()+"\", \"pwd\" : \""+pwd.toUtf8()+"\"}");
        write ( data );
    }
}

bool NetworkController::generate(const QVariantMap& data) {
    const QString id = data.value ( QLatin1String ( "id" ) ).toString();
    AbstractServiceProvider* service = m_servicestorage->get(id);
    bool remove = data.contains ( QLatin1String ( "remove" ) );

    if ( service )
    {
        QJson::QObjectHelper::qvariant2qobject ( data, service );
        if ( remove )
        {
            m_servicestorage->networkRemove(service, true);
            delete service;
            return true;
        }
        else
        {
            m_servicestorage->networkUpdate(service);
            return true;
        }
        return false;
    }
    else if ( remove || data.contains ( QLatin1String ( "iexecute" ) ) ) return false;

    // Object with id not known. Create new object
    const QString type = data.value ( QLatin1String ( "type" ), QString() ).toString();
    if ( type.isEmpty() )
    {
        qWarning() << __FUNCTION__ << "detected json object without type" << data;
        return false;
    }

    AbstractStateTracker* statetracker = 0;
    if (type.toAscii() == QByteArray(Category::staticMetaObject.className())) {
        service = new Category();
    } else if (type.toAscii() == QByteArray(Collection::staticMetaObject.className())) {
        service = new Collection();
    } else {
        ClientPlugin* eplugin = m_plugin_provider.value ( type );
        if ( !eplugin )
        {
            qWarning() << __FUNCTION__ << "no plugin for json object" << data;
            return false;
        }

        service=eplugin->base()->createServiceProvider ( type ) ;
        statetracker = eplugin->base()->createStateTracker(type);
    }

    if (service) {
        QJson::QObjectHelper::qvariant2qobject ( data, service );
        m_servicestorage->networkAdd(service);
    } else if (statetracker) {
        QJson::QObjectHelper::qvariant2qobject ( data, statetracker );
        m_servicestorage->networkUpdate(statetracker);
    } else {
        qWarning() << __FUNCTION__ << "no service/statetracker from plugin for json object" << data;
        return false;
    }

    return true;
}
void NetworkController::loadPlugins() {
    int offered_services = 0;
    ClientPlugin *plugin;
    QDir pluginsDir = QDir ( QLatin1String ( ROOM_SYSTEM_CLIENTPLUGINS ) );
    QStringList files = pluginsDir.entryList ( QDir::Files|QDir::NoDotAndDotDot );
    if (files.empty()) {
        qDebug() << "No plugins found in" << ROOM_SYSTEM_CLIENTPLUGINS;
    }
    foreach ( QString fileName, files )
    {
        QPluginLoader* loader = new QPluginLoader ( pluginsDir.absoluteFilePath ( fileName ), this );
        loader->setLoadHints(QLibrary::ResolveAllSymbolsHint);
        if (!loader->load()) {
            qWarning() << "Start: Failed loading" << pluginsDir.absoluteFilePath ( fileName ) << loader->errorString();
            delete loader;
            continue;
        }
        plugin = dynamic_cast<ClientPlugin*> ( loader->instance() );
        if (!plugin) {
            qWarning() << "Start: Failed to get instance" << pluginsDir.absoluteFilePath ( fileName );
            delete loader;
            continue;
        }
        qDebug() << "Start: Load Plugin"<<plugin->base()->name() <<plugin->base()->version();
	plugin->setStorages(m_servicestorage, m_modelstorage);
	plugin->init();

        m_plugins.append ( plugin );
        QStringList provides = plugin->base()->registerServices();
        offered_services += provides.size();
        provides.append(plugin->base()->registerStateTracker());
        foreach ( QString provide, provides )
        {
            m_plugin_provider.insert ( provide, plugin );
        }
    }
}

QStringList NetworkController::supportedPlugins() {
    return m_supportedPlugins;
}

QMap< QString, ClientPlugin* > NetworkController::plugin_providers() const {
    return m_plugin_provider;
}
