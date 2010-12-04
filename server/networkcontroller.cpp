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
#include <QSettings>
// Json
#include "shared/qjson/parser.h"
#include "shared/qjson/qobjecthelper.h"
#include "shared/qjson/serializer.h"

#include "shared/abstractserviceprovider.h"
#include "shared/abstractstatetracker.h"
#include "shared/server/executeservice.h"
#include "servicecontroller.h"
#include "authThread.h"
#include "config.h"

ClientConnection::ClientConnection(QSslSocket* s) {
    bufferpos=0;
    bufferBrakes=0;
    socket=s;
    m_auth=false;
    connect(&m_authTimer,SIGNAL(timeout()),SLOT(timeout()));
    m_authTimer.start(ROOM_NETWORK_AUTHTIMEOUT);
}
ClientConnection::~ClientConnection() {
    delete socket;
}
void ClientConnection::timeout() {
    emit timeoutAuth(socket);
}
bool ClientConnection::auth() {
    return m_auth;
}
void ClientConnection::setAuth(const QString& user) {
    m_user = user;
    m_auth = true;
    m_authTimer.stop();
}
QString ClientConnection::user() {
    return m_user;
}

///////////////////////////////////////////////////////////////////////////////

NetworkController::NetworkController(QDBusConnection dbusconnection)
        : m_dbusconnection(dbusconnection), m_service(0), m_auththread(new AuthThread())
{
    connect(m_auththread,SIGNAL(auth_failed(QObject*,QString)),SLOT(auth_failed(QObject*,QString)),Qt::QueuedConnection);
    connect(m_auththread,SIGNAL(auth_success(QObject*,QString)),SLOT(auth_success(QObject*,QString)),Qt::QueuedConnection);
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
    if ( !listen ( QHostAddress::Any, ROOM_LISTENPORT ) )
    {
        qCritical() << "TCP Server listen failed on port " << ROOM_LISTENPORT << "!";
        return false;
    } else
        qDebug() << "Listen on port" << ROOM_LISTENPORT;
    return true;
}

/*
   * Generate a self signed root certificate
    openssl req -x509 -utf8 -newkey rsa:1024 -nodes -days 3650 -keyout server.key -out server.crt
    openssl -new -x509 -extensions v3_ca -days 3650 -keyout cakey.pem -out cacert.pem
    openssl x509 -noout -text -in thecert.pem
*/
void NetworkController::incomingConnection ( int socketDescriptor )
{
    QSslSocket *socket = new QSslSocket;
    if ( socket->setSocketDescriptor ( socketDescriptor ) == true )
    {
        if (!QDir().exists(QLatin1String(ROOM_SYSTEM_CERTIFICATES"/server.crt"))) {
            qWarning()<<"Couldn't load local certificate"<<ROOM_SYSTEM_CERTIFICATES"/server.crt";
        } else
            socket->setLocalCertificate(QLatin1String(ROOM_SYSTEM_CERTIFICATES"/server.crt"));
        if (!QDir().exists(QLatin1String(ROOM_SYSTEM_CERTIFICATES"/server.key"))) {
            qWarning()<<"Couldn't load private key"<<ROOM_SYSTEM_CERTIFICATES"/server.key";
        } else
            socket->setPrivateKey(QLatin1String(ROOM_SYSTEM_CERTIFICATES"/server.key"));

        socket->setPeerVerifyMode(QSslSocket::VerifyNone);
        socket->startServerEncryption();
        connect ( socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
        connect(socket, SIGNAL(peerVerifyError(const QSslError &)),
                this, SLOT(peerVerifyError (const QSslError &)));
        connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)),
                this, SLOT(sslErrors(const QList<QSslError> &)));
		connect(socket,SIGNAL(disconnected()),SLOT(disconnected()));
        m_connections.insert ( socket, new ClientConnection(socket) );
        QByteArray data;
        data.append("{\"type\" : \"serverinfo\", \"version\" : \""ROOM_NETWORK_APIVERSION"\", \"auth\" : \"required\", \"auth_timeout\" : \"");
        data.append(QByteArray::number(ROOM_NETWORK_AUTHTIMEOUT));
        data.append("\", \"plugins\" : \"");
        data.append("\"}");
        socket->write ( data );
        //qDebug() << "new connection, waiting for authentification";
    }
    else
    {
        qWarning() << "Failed to bind incoming connection " << ROOM_LISTENPORT << "!";
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
        synclist.append(p->base());
    }
    const QList<AbstractStateTracker*> stateTrackerlist = m_service->stateTracker();
    foreach (AbstractStateTracker* p, stateTrackerlist)
    {
        synclist.append(p);
    }

    qDebug() << "New client from" << socket->peerAddress().toString().toUtf8().data() << "Sync objects:" << synclist.size();
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
    if (c->auth()) c->socket->write ( cmdbytes );
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

    foreach ( ClientConnection* c, m_connections ) {
		if (c->auth()) {
			c->socket->write ( cmdbytes );
		}
	}
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
        if (!c->auth()) {
            if (result.value(QLatin1String("type")).toByteArray()=="auth") {
                if (!m_auththread->query(socket, result.value(QLatin1String("name")).toString(),
                                         result.value(QLatin1String("pwd")).toString()
                                        )) {
                    QByteArray data("{\"type\" : \"auth\", \"result\" : \"notaccepted\"}");
                    socket->write ( data );
                }
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
    //qDebug() << "Client disconnected:" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor();
    m_connections.take ( socket )->deleteLater();
}


void NetworkController::log(const char* msg) {
    if (m_connections.isEmpty()) return;
    QByteArray cmdbytes("{\"type\" : \"log\", \"data\" : \"");
    // replace " with '
    for (char* msgr = (char*)msg;*msgr!=0;++msgr)
        if (*msgr == '"') *msgr = '\'';
    cmdbytes.append(msg);
    cmdbytes.append("\"}");
    foreach ( ClientConnection* c, m_connections )
    if (c->auth()) c->socket->write ( cmdbytes );
}
QDBusConnection* NetworkController::dbus() {
    return &m_dbusconnection;
}

void NetworkController::setServiceController ( ServiceController* controller ) {
    m_service = controller;
}

void NetworkController::auth_success(QObject* ptr, const QString& name) {
    if (ptr==0) { // Test
        qDebug() << __FUNCTION__ << name;
        return;
    }
    QSslSocket* socket = qobject_cast<QSslSocket*>(ptr);
    Q_ASSERT(socket);
    ClientConnection* c = m_connections.value(socket);
    Q_ASSERT(c);
    c->setAuth(name);
    QByteArray data("{\"type\" : \"auth\", \"result\" : \"ok\"}");
    socket->write ( data );
    syncClient(socket);
}

void NetworkController::auth_failed(QObject* ptr, const QString& name) {
    if (ptr==0) { // Test
        qDebug() << __FUNCTION__ << name;
        return;
    }
    Q_UNUSED(name);
    QSslSocket* socket = qobject_cast<QSslSocket*>(ptr);
    Q_ASSERT(socket);
    QByteArray data("{\"type\" : \"auth\", \"result\" : \"failed\"}");
    socket->write ( data );
    socket->flush();
}

void NetworkController::timeoutAuth(QSslSocket* socket) {
    QByteArray data("{\"type\" : \"auth\", \"result\" : \"timeout\"}");
    socket->write ( data );
    socket->flush();
    qWarning() << "Client auth timeout:" << socket->peerAddress().toString().toUtf8().data() << socket->socketDescriptor();
    m_connections.take ( socket )->deleteLater();
}

void NetworkController::sslErrors(QList< QSslError > errors) {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
    QString errorString;
    foreach(QSslError error, errors) {
		switch (error.error()) {
			default:
				errorString.append(error.errorString());
				errorString.append(QLatin1String(" "));
				break;
			case QSslError::SelfSignedCertificate:
				break;
		}
    }
    if (errorString.size())
		qWarning()<<__FUNCTION__ << socket->peerAddress()<< errorString;
}

void NetworkController::peerVerifyError(QSslError error) {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
	switch (error.error()) {
		default:
			qWarning()<<__FUNCTION__ << socket->peerAddress() << error.errorString();
			break;
		case QSslError::SelfSignedCertificate:
			break;
	}
}
