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
#include <QDateTime>
#include <QDebug>
// Json
#include "shared/qjson/parser.h"
#include "shared/qjson/qobjecthelper.h"
#include "shared/qjson/serializer.h"

#include <QSettings>
#include "shared/abstractserviceprovider.h"
#include "shared/abstractstatetracker.h"
#include "servicecontroller.h"
#include <shared/profile.h>
#include <shared/category.h>
#include "authThread.h"
#include "executeservice.h"

NetworkController::NetworkController(QDBusConnection dbusconnection)
        : m_dbusconnection(dbusconnection), m_service(0), m_auththread(new AuthThread())
{
	connect(m_auththread,SIGNAL(auth_failed(QObject*)),SLOT(auth_failed(QObject*)),Qt::QueuedConnection);
	connect(m_auththread,SIGNAL(auth_success(QObject*)),SLOT(auth_success(QObject*)),Qt::QueuedConnection);
    m_auththread->start();
}

NetworkController::~NetworkController()
{
    qDeleteAll(m_connections);
    m_connections.clear();
    close();
}

bool NetworkController::start()
{
    if ( !listen ( QHostAddress::Any, LISTENPORT ) )
    {
        qCritical() << "TCP Server listen failed on port " << LISTENPORT << "!";
        return false;
    } else
        qDebug() << "Listen on port" << LISTENPORT;
    return true;
}

void NetworkController::incomingConnection ( int socketDescriptor )
{
    QSslSocket *socket = new QSslSocket;
    if ( socket->setSocketDescriptor ( socketDescriptor ) == true )
    {
        connect ( socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
        m_connections.insert ( socket, new ClientConnection(socket) );
        qDebug() << "new connection, waiting for authentification";
    }
    else
    {
        qWarning() << "Failed to bind incoming connection " << LISTENPORT << "!";
        delete socket;
    }
}

void NetworkController::syncClient(QSslSocket* socket)
{
    //sync all providers with the new client
    QList<QObject*> synclist;

    const QList<ExecuteWithBase*> servicesList = m_service->m_servicesList;
	foreach (ExecuteWithBase* p, servicesList)
    {
        synclist.append(p->baseService());
    }
    const QList<AbstractStateTracker*> stateTrackerlist = m_service->stateTracker();
    foreach (AbstractStateTracker* p, stateTrackerlist)
    {
        synclist.append(p);
    }

    qDebug() << "New client from" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor() << "sync objects:" << synclist.size();
    foreach (QObject* p, synclist)
    {
        QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
        QJson::Serializer serializer;
        QByteArray cmdbytes = serializer.serialize(variant);
        socket->write ( cmdbytes );
    }
    socket->write("{\"type\" : \"complete\"}");
    socket->flush();
}

QByteArray NetworkController::getNextJson(ClientConnection* c)
{
    for (; c->bufferpos<c->buffer.size(); ++c->bufferpos)
    {
        if (c->buffer[c->bufferpos] == '{')
            ++c->bufferBrakes;
        else if (c->buffer[c->bufferpos] == '}')
            --c->bufferBrakes;

        if (c->bufferpos>1 && c->bufferBrakes == 0)
        {
            const QByteArray res = c->buffer.mid(0,c->bufferpos+1);
            c->buffer.remove(0,c->bufferpos+1);
            c->bufferpos = 0;
            return res;
        }
    }

    // no paragraph {..} found
    return QByteArray();
}

void NetworkController::statetrackerSync(AbstractStateTracker* p)
{
	// do nothing if no clients are connected
	if (m_connections.isEmpty()) return;
	
	QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
	QJson::Serializer serializer;
	QByteArray cmdbytes = serializer.serialize(variant);
	
	foreach ( ClientConnection* c, m_connections )
		c->socket->write ( cmdbytes );
}

void NetworkController::serviceSync(AbstractServiceProvider* p)
{
    // do nothing if no clients are connected
    if (m_connections.isEmpty()) return;

    QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
    if (p->dynamicPropertyNames().contains("remove"))
        variant.insert(QLatin1String("remove"), true);
    if (p->dynamicPropertyNames().contains("iexecute"))
        variant.insert(QLatin1String("iexecute"), true);
    QJson::Serializer serializer;
    QByteArray cmdbytes = serializer.serialize(variant);

    foreach ( ClientConnection* c, m_connections )
        c->socket->write ( cmdbytes );
}

void NetworkController::readyRead()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    Q_ASSERT(socket);
    ClientConnection* c = m_connections.value(socket);
    if (!c) return;

    if ( socket->bytesAvailable() )
    {
        c->buffer += socket->readAll();
        // if buffer>2MByte discard
        if ( c->buffer.size() >1024*2000 )
        {
            c->buffer.clear();
            c->bufferBrakes = 0;
            c->bufferpos = 0;
            qWarning() << "receive incomplete" << c->buffer;
            return;
        }
    } else return;


    QByteArray cmdstring;
    while (1) {
        cmdstring = getNextJson(c);
        if (cmdstring.isEmpty())
        {
            return;
        }

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse (cmdstring, &ok).toMap();
        if (!ok || !result.contains(QLatin1String("type"))) {
            qWarning() << "could not parse cmd" << ok << cmdstring;
            continue;
        }
        if (!c->auth) {
            if (result.value(QLatin1String("type")).toByteArray()=="auth") {
                m_auththread->query(socket, result.value(QLatin1String("name")).toString(),
                                    result.value(QLatin1String("pwd")).toString()
                                   );
            } else {
                QByteArray data("{\"type\" : \"auth\", \"result\" : \"missing\"}");
                socket->write ( data );
            }
			continue;
		}
        // Let the factory update existing objects or create new ones if necessary
        m_service->generate(result);

    };
}

void NetworkController::disconnected()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    Q_ASSERT(socket);
    m_connections.remove ( socket );
    qDebug() << "Client disconnected:" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor();
    socket->deleteLater();
}


void NetworkController::log(const char* msg) {
    if (m_connections.isEmpty()) return;
    QByteArray data("{\"type\" : \"log\", \"data\" : \"");
    // replace " with '
    for (char* msgr = (char*)msg;*msgr!=0;++msgr)
        if (*msgr == '"') *msgr = '\'';
    data.append(msg);
    data.append("\"}");
    foreach ( ClientConnection* c, m_connections )
    {
        c->socket->write ( data );
    }
}
QDBusConnection* NetworkController::dbus() {
    return &m_dbusconnection;
}

void NetworkController::setServiceController ( ServiceController* controller ) {
    m_service = controller;
}

void NetworkController::auth_success(QObject* ptr) {
    QSslSocket* socket = qobject_cast<QSslSocket*>(ptr);
	Q_ASSERT(socket);
	ClientConnection* c = m_connections.value(socket);
	Q_ASSERT(c);
	c->auth = true;
	QByteArray data("{\"type\" : \"auth\", \"result\" : \"ok\"}");
	socket->write ( data );
    syncClient(socket);
}

void NetworkController::auth_failed(QObject* ptr) {
    QSslSocket* socket = qobject_cast<QSslSocket*>(ptr);
	Q_ASSERT(socket);
	QByteArray data("{\"type\" : \"auth\", \"result\" : \"failed\"}");
	socket->write ( data );
	socket->flush();
}
