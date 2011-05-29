/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Purpose: https server
*/

// QT
#include <QDebug>
#include <QUrl>
// json
#include <serializer.h>
#include <parser.h>

#include "paths.h"
#include "config.h"
#include "websocket.h"
#include "sessioncontroller.h"
#include "httprequest.h"
#include "httpserver.h"

WebSocket* WebSocket::makeWebsocket(HttpRequest* request, QObject* parent) {
    if (request->m_header.value("Upgrade").toLower() != "websocket" || request->m_header.value("Connection") != "Upgrade")
        return 0;

    if (request->m_header.value("Sec-WebSocket-Protocol") != "roomcontrol") {
        qWarning()<<"Websocket: Wrong subprotocol";
        return 0;
    }
    if (request->m_header.contains("Sec-WebSocket-Key")) {
        //Version 4+
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(request->m_header.value("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

        request->m_socket->write("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: roomcontrol\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("Sec-WebSocket-Accept: "+hash.result().toBase64()+"\r\n\r\n");
        request->m_socket->flush();
    } else if (request->m_header.contains("Sec-WebSocket-Key1") && request->m_header.contains("Sec-WebSocket-Key2") && request->m_socket->bytesAvailable()>=8) {
        //Version 0-3
        const QByteArray origin = request->m_header.value("Origin");
        if (origin.isEmpty() ) {
            qWarning()<<"Websocket: Host or Origin not set!"<<request->m_header;
            return 0;
        }
        QByteArray s1 = request->m_header.value("Sec-WebSocket-Key1");
        QByteArray s2 = request->m_header.value("Sec-WebSocket-Key2");
        QByteArray sn1, sn2;
        int count1 = 0, count2 = 0;
        for (int i=0;i<s1.count();++i) {
            if (s1[i] == ' ') count1++;
            if (QChar::fromAscii(s1[i]).isDigit()) sn1 += s1[i];
        }
        for (int i=0;i<s2.count();++i) {
            if (s2[i] == ' ') count2++;
            if (QChar::fromAscii(s2[i]).isDigit()) sn2 += s2[i];
        }
        if (count1==0 || count2==0) {
            qWarning()<<"Websocket: Protocol < 4 Key Handshake failed. Wrong spaces.";
            return 0;
        }
        unsigned char sum[16];
        memcpy(&sum[8], request->m_socket->read(8).constData(), 8);
        int32_t number1 = sn1.toUInt() / count1;
        int32_t number2 = sn2.toUInt() / count2;
        sum[0] = number1 >> 24;
        sum[1] = number1 >> 16;
        sum[2] = number1 >> 8;
        sum[3] = number1;
        sum[4] = number2 >> 24;
        sum[5] = number2 >> 16;
        sum[6] = number2 >> 8;
        sum[7] = number2;

        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData((char*)sum,sizeof(sum));

        request->m_socket->write("HTTP/1.1 101 Switching Protocols\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: roomcontrol\r\n");
        request->m_socket->write("sec-WebSocket-Origin: "+origin+"\r\n");
        QByteArray location = request->m_header.value("Host");
        if (location.isEmpty()) location = request->m_socket->localAddress().toString().toAscii()+":"+QByteArray::number(request->m_socket->localPort());
        request->m_socket->write("Sec-WebSocket-Location: wss://"+location+"/"+QFileInfo(request->m_requestedfile).fileName().toLatin1()+"\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("\r\n"+hash.result());
        request->m_socket->flush();
    } else {
        qWarning()<<"Websocket: Draft Version not recognized. Server supports draft-ietf-hybi-thewebsocketprotocol 00-06";
        return 0;
    }

    WebSocket* w = new WebSocket(parent);
    w->setSocket(request->takeOverSocket());
    w->m_sessionid = request->m_sessionid;
    return w;
}

WebSocket::WebSocket(QObject* parent) :QObject(parent), m_socket(0) {
}

WebSocket::~WebSocket() {
    const char data[] = {0x00, 0xFF, 0x00, 0xFF};
    m_socket->write(data);
    m_socket->flush();
    m_socket->blockSignals(true);
    m_socket->close();
    delete m_socket;
}

void WebSocket::setSocket(QSslSocket* socket) {
    Q_ASSERT(socket);
    m_socket = socket;
    connect(m_socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
    connect(m_socket, SIGNAL(peerVerifyError(const QSslError &)),
            this, SLOT(peerVerifyError (const QSslError &)));
    connect(m_socket, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(sslErrors(const QList<QSslError> &)));
    connect(m_socket,SIGNAL(disconnected()),SLOT(disconnected()));
}

void WebSocket::readyRead()
{
    if (m_socket->bytesAvailable() > 1024*2000) {
        m_socket->readAll();
        qWarning() << "receive incomplete";
        return;
    }

    int frameIndex;
    while (1) {
        // read until framestart is the first byte (read that one too)
        frameIndex = m_socket->peek(1000).indexOf(char(0x00));
        if (frameIndex==-1) return; // No frame start, wait for new data
        if (frameIndex>0) m_socket->read(frameIndex); // read everything until framestart

        // find frameend
        frameIndex = m_socket->peek(1000).indexOf(char(0xFF));
        if (frameIndex==-1) return; // No frame end, wait for new data
        QByteArray frame = m_socket->read(frameIndex+1); // read everything until framestart

        // remove websocket frame bytes
        frame.chop(1);
        frame = frame.mid(1).trimmed();

        if (!frame.startsWith("{") || !frame.endsWith("}") ) {
            qWarning()<<"Websocket: Frame received but does not match subprotocol roomcontrol";
            emit removeWebSocket(this);
            return;
        }
        bool ok = true;
        const QVariantMap data = QJson::Parser().parse (frame, &ok).toMap();
        if (!ok) continue;
        int i = SessionController::instance()->tryLogin(data, m_sessionid);
        switch (i) {
        case 0:
            emit dataReceived(data, m_sessionid);
            break;
        case 1:
            break;
        case 2:
			emit gotSession(this, m_sessionid);
            break;
        default:
            break;
        }
    }
}

void WebSocket::sslErrors(QList< QSslError > errors) {
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
        qWarning()<<__FUNCTION__ << m_socket->peerAddress()<< errorString;
}

void WebSocket::peerVerifyError(QSslError error) {
    switch (error.error()) {
    default:
        qWarning()<<__FUNCTION__ << m_socket->peerAddress() << error.errorString();
        break;
    case QSslError::SelfSignedCertificate:
        break;
    }
}

void WebSocket::disconnected()
{
    emit removeWebSocket(this);
}

void WebSocket::writeJSON(const QByteArray& data) {
    m_socket->write(char(0x00)+data+"\n"+char(0xFF));
}
QString WebSocket::getSessionID() {
    return m_sessionid;
}
