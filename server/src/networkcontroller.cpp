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
#include "qjson/parser.h"
#include "qjson/qobjecthelper.h"
#include "qjson/serializer.h"

#include "RoomControlServer.h"
#include "abstractserviceprovider.h"
#include "factory.h"
#include "stateTracker/abstractstatetracker.h"
#include <QSettings>
#include "curtaincontroller.h"
#include "ledcontroller.h"

NetworkController::NetworkController() {}

NetworkController::~NetworkController()
{
    qDeleteAll(connections);
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
    QTcpSocket* socket = new QTcpSocket ( this );
    if ( socket->setSocketDescriptor ( socketDescriptor ) == true )
    {
        connect ( socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
        connections.insert ( socket, new ClientConnection(socket) );
        syncClient(socket);
    }
    else
    {
        qWarning() << "Failed to bind incoming connection " << LISTENPORT << "!";
        delete socket;
    }
}

void NetworkController::syncClient(QTcpSocket* socket)
{
    if (!RoomControlServer::getFactory()) {
        qWarning() << "Client tries to connect too early. Factory not ready!";
        return;
    }

    //sync all providers with the new client
    QList<QObject*> synclist;
    const QList<AbstractServiceProvider*> providerlist = RoomControlServer::getFactory()->m_providerList;
    foreach (AbstractServiceProvider* p, providerlist)
    {
        synclist.append(p);
    }
    const QList<AbstractStateTracker*> stateTrackerlist = RoomControlServer::getRoomControlServer()->getStateTracker();
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

void NetworkController::objectSync(QObject* p)
{
    // do nothing if no clients are connected
    if (connections.isEmpty()) return;

    QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
    if (p->dynamicPropertyNames().contains("remove"))
        variant.insert(QLatin1String("remove"), true);
    QJson::Serializer serializer;
    QByteArray cmdbytes = serializer.serialize(variant);

    foreach ( ClientConnection* c, connections )
    {
        c->socket->write ( cmdbytes );
    }
}

void NetworkController::readyRead()
{
    QTcpSocket* socket = ( QTcpSocket* ) sender();
    ClientConnection* c = connections.value(socket);
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
        const QString type = result.value(QLatin1String("type")).toString();
        // Special commandos; no provider for them
        if (type == QLatin1String("quit"))
        {
            // quit program
            QCoreApplication::exit(0);
            return;
        } else if (type == QLatin1String("restart"))
        {
            // restart the application
            // (e.g. if a new version of the executable is available)
            QCoreApplication::exit(1);
            return;
        } else if (type == QLatin1String("refresh"))
        {
            // request all values from the ethersex again
            RoomControlServer::getCurtainController()->refresh();
            RoomControlServer::getLedController()->refresh();
            continue;
        } else if (type == QLatin1String("resync"))
        {
            syncClient(socket);
            continue;
        } else if (type == QLatin1String("backup"))
        {
            RoomControlServer::getFactory()->backup();
            continue;
        } else if (type == QLatin1String("backup_restore"))
        {
            RoomControlServer::getFactory()->backup_restore(result.value(QLatin1String("id")).toString());
            continue;
        } else if (type == QLatin1String("backup_remove"))
        {
            RoomControlServer::getFactory()->backup_remove(result.value(QLatin1String("id")).toString());
            continue;
        } else if (type == QLatin1String("backup_list"))
        {
            QByteArray data("{\"type\" : \"backup_list\", \"data\" : \"");
            data.append(RoomControlServer::getFactory()->backup_list().join(QLatin1String("|")).toUtf8());
            data.append("\"}");
            socket->write(data);

            continue;
        }

        // Let the factory update existing objects or create new ones if necessary
        RoomControlServer::getFactory()->examine(result);

    };
}

void NetworkController::disconnected()
{
    QTcpSocket* socket = ( QTcpSocket* ) sender();
    connections.remove ( socket );
    qDebug() << "Client disconnected:" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor();
    socket->deleteLater();
}

void NetworkController::backup_list_changed() {
    QByteArray data("{\"type\" : \"backup_list\", \"data\" : \"");
    data.append(RoomControlServer::getFactory()->backup_list().join(QLatin1String("|")).toUtf8());
    data.append("\"}");
    foreach ( ClientConnection* c, connections )
    {
        c->socket->write ( data );
    }
}

void NetworkController::log(const char* msg) {
    QByteArray data("{\"type\" : \"log\", \"data\" : \"");
    // replace " with '
    for (char* msgr = (char*)msg;*msgr!=0;++msgr)
        if (*msgr == '"') *msgr = '\'';
    data.append(msg);
    data.append("\"}");
    foreach ( ClientConnection* c, connections )
    {
        c->socket->write ( data );
    }
}
