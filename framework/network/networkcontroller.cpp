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
#include <QDebug>
// Json
#include "qjson/parser.h"
#include "qjson/qobjecthelper.h"
#include "qjson/serializer.h"

#include "RoomControlClient.h"
#include "abstractserviceprovider.h"
#include "factory.h"
#include "stateTracker/abstractstatetracker.h"

#include <solid/networking.h>

NetworkController::NetworkController()
{
    m_bufferpos=0;
    m_bufferBrakes=0;
    connect(&serverTimeout,SIGNAL(timeout()),SLOT(timeout()));
    serverTimeout.setSingleShot(true);
    serverTimeout.setInterval(5000);

    connect ( this, SIGNAL ( readyRead() ), SLOT ( slotreadyRead() ) );
    connect ( this, SIGNAL ( connected() ), SLOT( slotconnected()) );
    connect ( this, SIGNAL ( disconnected() ), SLOT( slotdisconnected()) );
    connect ( this, SIGNAL ( error(QAbstractSocket::SocketError) ), SLOT( sloterror(QAbstractSocket::SocketError) ));
	
	connect( Solid::Networking::notifier(), SIGNAL(shouldDisconnect()), SLOT(timeout()) );
}

NetworkController::~NetworkController()
{
    disconnectFromHost();
}

void NetworkController::slotconnected()
{
    qDebug() << "Connected to"<<peerAddress().toString()<<peerPort();
    serverTimeout.start();
}

void NetworkController::slotdisconnected()
{
	
}

void NetworkController::timeout()
{
    disconnectFromHost();
}

void NetworkController::sloterror(QAbstractSocket::SocketError e)
{
    qWarning() << __FUNCTION__ << e;
}


void NetworkController::start(const QString& ip, int port)
{
	m_ip = ip;
	m_port = port;
    connectToHost ( ip, port );
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

void NetworkController::objectSync(QObject* p)
{
    // do nothing if not connected
    if (!isWritable()) return;

    QVariantMap variant = QJson::QObjectHelper::qobject2qvariant(p);
    if (p->dynamicPropertyNames().contains("remove"))
      variant.insert(QLatin1String("remove"), true);

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
        if (!ok || !result.contains(QLatin1String("type"))) {
            qWarning() << "could not parse cmd" << cmdstring;
            continue;
        }

	if (result.value(QLatin1String("type"))=="complete") {
	    RoomControlClient::getFactory()->syncComplete();
	    continue;
	}
        else if (result.value(QLatin1String("type"))==ProgramStateTracker::staticMetaObject.className())
        {
            QJson::QObjectHelper::qvariant2qobject(result, &m_serverstate);
            if (m_serverstate.minversion().toAscii()>=NETWORK_MIN_APIVERSION &&
                    m_serverstate.maxversion().toAscii()<=NETWORK_MAX_APIVERSION)
            {
                serverTimeout.stop();
                emit connectedToValidServer();
		RoomControlClient::getFactory()->syncStarted();
            }
            continue;
        }

        RoomControlClient::getFactory()->examine(result);
    };
}

QString NetworkController::serverversion()
{
    return m_serverstate.appversion();
}

void NetworkController::resync()
{
    const QByteArray cmd = "{\"type\" : \"resync\"}";
    write(cmd);
}

void NetworkController::refresh()
{
    const QByteArray cmd = "{\"type\" : \"refresh\"}";
    write(cmd);
}

void NetworkController::restart()
{
    const QByteArray cmd = "{\"type\" : \"restart\"}";
    write(cmd);
}
