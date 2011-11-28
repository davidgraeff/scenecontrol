#include "websocket.h"
#include "libwebsocket/libwebsockets.h"
#include "paths.h"
#include "config.h"
#include <QDebug>
#include <QSslKey>
#include <parser.h>
#include "libwebsocket/private-libwebsockets.h"

#define __FUNCTION__ __FUNCTION__

enum libwebsocket_protocols_enum {
    /* always first */
    PROTOCOL_HTTP = 0,

    PROTOCOL_ROOMCONTROL,

    /* always last */
    PROTOCOL_COUNT
};


static int callback_http(struct libwebsocket_context * context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
    Q_UNUSED(context);
    Q_UNUSED(wsi);
    Q_UNUSED(reason);
    Q_UNUSED(user);
    Q_UNUSED(in);
    Q_UNUSED(len);

    switch (reason) {
    case LWS_CALLBACK_ADD_POLL_FD:
// 		pollfds[count_pollfds].fd = (int)(long)user;
// 		pollfds[count_pollfds].events = (int)len;
// 		pollfds[count_pollfds++].revents = 0;
        ((WebSocket*)context->cpp_class)->addWebsocketFD((int)(long)user, (int)len);
        break;

    case LWS_CALLBACK_DEL_POLL_FD:
        ((WebSocket*)context->cpp_class)->removeWebsocketFD((int)(long)user);
// 		for (n = 0; n < count_pollfds; n++)
// 			if (pollfds[n].fd == (int)(long)user)
// 				while (n < count_pollfds) {
// 					pollfds[n] = pollfds[n + 1];
// 					n++;
// 				}
// 		count_pollfds--;

        break;

    case LWS_CALLBACK_SET_MODE_POLL_FD:
// 		for (n = 0; n < count_pollfds; n++)
// 			if (pollfds[n].fd == (int)(long)user)
// 				pollfds[n].events |= (int)(long)len;
        break;

    case LWS_CALLBACK_CLEAR_MODE_POLL_FD:
// 		for (n = 0; n < count_pollfds; n++)
// 			if (pollfds[n].fd == (int)(long)user)
// 				pollfds[n].events &= ~(int)(long)len;
        break;

    default:
        break;
    }

    return 0;
}


static int callback_roomcontrol_protocol(struct libwebsocket_context * context,
        struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
    Q_UNUSED(context);
    Q_UNUSED(wsi);
    Q_UNUSED(user);
    int n;
    switch (reason) {
    case LWS_CALLBACK_BROADCAST:
        n = libwebsocket_write(wsi, &((unsigned char*)in)[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT);
        if (n < 0) {
            fprintf(stderr, "ERROR writing to socket");
            return 1;
        }
// 		if (close_testing && pss->number == 50) {
// 			fprintf(stderr, "close tesing limit, closing\n");
// 			libwebsocket_close_and_free_session(context, wsi,
// 						       LWS_CLOSE_STATUS_NORMAL);
// 		}
        break;
    case LWS_CALLBACK_RECEIVE:
        ((WebSocket*)context->cpp_class)->websocketReceive(QByteArray::fromRawData((char*)in, len), wsi);
        break;
    default:
        break;
    }

    return 0;
}

static struct libwebsocket_protocols protocols[] = {
    /* first protocol must always be HTTP handler */

    {
        "http-only",		/* name */
        callback_http,		/* callback */
        0,			/* per_session_data_size */
        0,0,0,0
    },
    {
        "roomcontrol-protocol",
        callback_roomcontrol_protocol,
        0,
        0,0,0,0
    },
    {
        NULL, NULL, 0,		/* End of list */
        0,0,0,0
    }
};

WebSocket::~WebSocket()
{
    libwebsocket_context_destroy(m_websocket_context);
    m_websocket_context = 0;
    qDeleteAll(m_websocket_fds);
}

WebSocket::WebSocket() : m_websocket_context( 0 ) {
    m_websocket_context = libwebsocket_create_context(ROOM_LISTENPORT, 0, protocols,
                          libwebsocket_internal_extensions,
                          setup::certificateFile("server.crt").toLatin1().constData(), setup::certificateFile("server.key").toLatin1().constData(), -1, -1, 0, this);
    if (m_websocket_context == 0) {
        qWarning() << "libwebsocket init failed";
    } else {
        for (int i=0; i< m_websocket_context->fds_count; ++i) {
            addWebsocketFD(m_websocket_context->fds[i].fd, m_websocket_context->fds[i].events);
        }
        qDebug() << "SSL Websocket Server ready on port" << ROOM_LISTENPORT;
    }

    if (listen(QHostAddress::Any, ROOM_LISTENPORT+1)) {
        qDebug() << "SSL TCPSocket Server ready on port" << ROOM_LISTENPORT+1;
    } else {
        qWarning() << "TCPSocket Server init failed";
    }
}
static WebSocket* websocket_instance = 0;
WebSocket* WebSocket::instance()
{
    if (!websocket_instance)
        websocket_instance = new WebSocket();
    return websocket_instance;
}

void WebSocket::incomingConnection(int socketDescriptor)
{
    qDebug() << "new socket" << socketDescriptor;
    QSslSocket *socket = new QSslSocket;
    if (socket->setSocketDescriptor(socketDescriptor)) {
        m_sockets.insert(socketDescriptor, socket);
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(socket, SIGNAL(sslErrors (QList<QSslError>)), this, SLOT(sslErrors (QList<QSslError>)));
        socket->ignoreSslErrors();
        socket->setProtocol(QSsl::SslV3);

        QByteArray key;
        QByteArray cert;

        QFile fileKey(setup::certificateFile("server.key"));
        if(fileKey.open(QIODevice::ReadOnly))
        {
            key = fileKey.readAll();
            fileKey.close();
        }
        else
        {
            qWarning() << fileKey.errorString();
        }

        QFile fileCert(setup::certificateFile("server.crt"));
        if(fileCert.open(QIODevice::ReadOnly))
        {
            cert = fileCert.readAll();
            fileCert.close();
        }
        else
        {
            qWarning() << fileCert.errorString();
        }

        QSslKey sslKey(key, QSsl::Rsa);
        if (key.isNull()) {
            qWarning() << "key invalid";
        }

        QSslCertificate sslCert(cert);
        if (sslCert.isNull()) {
            qWarning() << "sslCert invalid";
        }

        socket->setPrivateKey(sslKey);
        socket->setLocalCertificate(sslCert);
        socket->startServerEncryption();
        qDebug() << "new socket" << socketDescriptor << "enc" << socket->sslErrors() << socket->peerAddress() << socket->state();
    } else {
        delete socket;
    }
}

void WebSocket::sslErrors ( const QList<QSslError> & errors ) {
    qDebug() << errors;
}

void WebSocket::websocketactivity(int) {
    libwebsocket_service(m_websocket_context,0);
}
void WebSocket::addWebsocketFD(int fd, short int direction) {
    if (m_websocket_fds.contains(fd))
        return;
    QSocketNotifier::Type _direction = ((direction == POLLOUT) ? QSocketNotifier::Write : QSocketNotifier::Read);
    QSocketNotifier* sn = new QSocketNotifier(fd, _direction, this);
    connect(sn, SIGNAL(activated(int)), SLOT(websocketactivity(int)));
    sn->setEnabled(true);
    m_websocket_fds.insert(fd, sn);
}

void WebSocket::removeWebsocketFD(int fd) {
    delete m_websocket_fds.take(fd);
}

void WebSocket::readyRead() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
    bool ok;
    while (serverSocket->canReadLine()) {
        const QByteArray rawdata = serverSocket->readLine();
        qDebug() << "socket read" << serverSocket->socketDescriptor() << rawdata.length() << rawdata.toBase64();
        if (!rawdata.length())
            continue;
        QVariant v = QJson::Parser().parse(rawdata, &ok);
        if (ok)
            emit requestExecution(v.toMap(), serverSocket->socketDescriptor());
    }
}

void WebSocket::socketDisconnected() {
    QSslSocket *socket = (QSslSocket *)sender();
    m_sockets.remove(socket->socketDescriptor());
    socket->deleteLater();
    qDebug() << "socket closed" << socket->socketDescriptor() << socket->errorString() << socket->error();
}

void WebSocket::websocketReceive(const QByteArray& rawdata, libwebsocket* wsi) {
    bool ok;
    QList<QByteArray> list = rawdata.split('\n');
    for (int i=0;i<list.size(); ++i) {
        QVariant v = QJson::Parser().parse(rawdata, &ok);
        if (ok)
            emit requestExecution(v.toMap(), wsi->sock);
    }
}

void WebSocket::sendToAllClients(const QByteArray& rawdata) {
    if (!m_websocket_context)
        return;
    // send data over websocket. First convert to json string then copy to a c buffer and send to all connected websocket clients with the right protocol
    const int len = rawdata.size();
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + len];
    memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING,rawdata.constData(),len);
    libwebsockets_broadcast(&protocols[PROTOCOL_ROOMCONTROL], buf, len);
    // send data over sockets
    QMap<int, QSslSocket*>::const_iterator it = m_sockets.constBegin();
    while(it != m_sockets.constEnd()) {
        it.value()->write(rawdata);
    }
}

void WebSocket::sendToClient(const QByteArray& rawdata, int sessionid) {
    // try to find a socket connection with corresponding sessionid
    QSslSocket *socket = m_sockets.value(sessionid);
    if (socket) {
        socket->write(rawdata);
        return;
    }

    // try to find a websocket connection with corresponding sessionid
    if (!m_websocket_context)
        return;

    libwebsocket* wsi = wsi_from_fd(m_websocket_context, sessionid);
    if (!wsi) {
      qWarning() << "Websocket: sessionid not found!" << sessionid << wsi;
        return;
    }
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + rawdata.size()];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    memcpy(p,rawdata.constData(),rawdata.size());
    int n = libwebsocket_write(wsi, p, rawdata.size(), LWS_WRITE_TEXT);
    if (n < 0) {
        qWarning() << "ERROR writing to socket: websocketClientRequestAllProperties";
    }
}
