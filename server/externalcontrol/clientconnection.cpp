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
#include <QtNetwork/QHostAddress>
#include <QStringList>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QSettings>

#include "paths.h"
#include "servicecontroller.h"
#include "authThread.h"
#include "config.h"
#include <serializer.h>
#include <parser.h>
#include "clientconnection.h"
#include <sessioncontroller.h>
#include <QUrl>
#include <stdint.h>
#include "httpserver.h"


ClientConnection::ClientConnection(HttpServer* server, QSslSocket* s) :m_server(server), m_socket(s) {
    connect(m_socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
    connect(m_socket, SIGNAL(peerVerifyError(const QSslError &)),
            this, SLOT(peerVerifyError (const QSslError &)));
    connect(m_socket, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(sslErrors(const QList<QSslError> &)));
    connect(m_socket,SIGNAL(disconnected()),SLOT(disconnected()));

    connect(&m_timeout, SIGNAL(timeout()), SLOT(timeout()));
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(5*60*1000);
    m_timeout.start();

    m_isWebsocket = false;
    m_inHeader = false;
}
ClientConnection::~ClientConnection() {
    if (m_isWebsocket) {
        const char data[] = {0x00, 0xFF, 0x00, 0xFF};
        m_socket->write(data);
        m_socket->flush();
    }
    m_socket->blockSignals(true);
    m_socket->close();
    delete m_socket;
}

void ClientConnection::readyRead()
{
    // disconnect socket after timeout
    m_timeout.stop();
    if (!m_authok) m_timeout.start();

    if (m_socket->bytesAvailable() > 1024*2000) {
        m_socket->readAll();
        qWarning() << "receive incomplete";
        return;
    }

    if (m_isWebsocket) {
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
                emit removeConnection(this);
                return;
            }
            bool ok = true;
            const QVariantMap data = QJson::Parser().parse (frame, &ok).toMap();
            if (!ok) continue;
            if (ServiceID::isId(data,"sessionlogin")) { // login have to happen here
                sessionid = SessionController::instance()->addSession(DATA("user"),DATA("pwd"));
            } else if (ServiceID::isId(data,"sessionidle")) {
                Session* session = SessionController::instance()->getSession(sessionid);
                Q_ASSERT(session);
                session->resetSessionTimer();
            }
            else emit dataReceived(data, sessionid);
        }

    } else
        while (m_socket->canReadLine()) {
            if (!readHttp(m_socket->readLine().trimmed())) {
                return;
            }
        }
}

void ClientConnection::sslErrors(QList< QSslError > errors) {
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

void ClientConnection::peerVerifyError(QSslError error) {
    switch (error.error()) {
    default:
        qWarning()<<__FUNCTION__ << m_socket->peerAddress() << error.errorString();
        break;
    case QSslError::SelfSignedCertificate:
        break;
    }
}

void ClientConnection::disconnected()
{
    emit removeConnection(this);
}

void ClientConnection::sessionEstablished() {
    m_authok=true;
}

void ClientConnection::writeJSON(const QByteArray& data) {
    m_socket->write(char(0x00)+data+"\n"+char(0xFF));
}

void ClientConnection::timeout() {
    emit removeConnection(this);
}

void ClientConnection::generateFileResponse() {
    QFile www(m_requestedfile);
    QFileInfo wwwinfo(www);
    if (wwwinfo.exists()) {
        if (!wwwinfo.isReadable()) {
            m_socket->write("HTTP/1.1 403 Forbidden\r\n");
            m_socket->write("Content-Length: 0\r\n");
            writeDefaultHeaders();
            m_socket->write("\r\n");
            m_socket->flush();
            return;
        }
        m_socket->write("HTTP/1.1 200 OK\r\n");
        writeDefaultHeaders();

        QByteArray type = "text/html";
        if (wwwinfo.suffix()==QLatin1String("gif")||wwwinfo.suffix()==QLatin1String("png")) {
            type = "image/"+wwwinfo.suffix().toAscii();
        } else if (wwwinfo.suffix()==QLatin1String("ico")) {
            type = "image/vnd.microsoft.icon";
        } else if (wwwinfo.suffix()==QLatin1String("jpg")||wwwinfo.suffix()==QLatin1String("jpeg")) {
            type = "image/jpeg";
        } else if (wwwinfo.suffix()==QLatin1String("css")) {
            type = "text/css";
        } else if (wwwinfo.suffix()==QLatin1String("js")) {
            type = "application/x-javascript";
        } else if (wwwinfo.suffix()==QLatin1String("xml")) {
            type = "text/xml";
        }

        m_socket->write("Last-Modified: " + wwwinfo.lastModified().toString(QLatin1String("ddd, d MMMM yyyy hh:mm:ss")).toAscii() + " GMT\r\n");
        m_socket->write("Content-Type: " + type + "\r\n");
        m_socket->write("Content-Length:"+QByteArray::number(wwwinfo.size())+"\r\n");

        m_socket->write("\r\n");

        if (m_requestType==Get || m_requestType==Post) {
            www.open(QIODevice::ReadOnly);
            m_socket->write(www.readAll());
            www.close();
        }
        m_socket->flush();
    } else {
        qWarning() << "File not found: " << wwwinfo.absoluteFilePath();
        m_socket->write("HTTP/1.1 404 Not Found\r\n");
        m_socket->write("Content-Length: 0\r\n");
        writeDefaultHeaders();
        m_socket->write("\r\n");
        m_socket->flush();
    }
}

void ClientConnection::generateWebsocketResponseV04() {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(m_header.value("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    m_socket->write("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: roomcontrol\r\n");
    m_socket->write("Sec-WebSocket-Accept: "+hash.result().toBase64()+"\r\n\r\n");
    m_socket->flush();
}

void ClientConnection::generateWebsocketResponseV00() {
    const QByteArray origin = m_header.value("Origin");
    if (origin.isEmpty() ) {
        qWarning()<<"Websocket: Host or Origin not set!"<<m_header;
        emit removeConnection(this);
        return;
    }
    QByteArray s1 = m_header.value("Sec-WebSocket-Key1");
    QByteArray s2 = m_header.value("Sec-WebSocket-Key2");
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
        emit removeConnection(this);
        return;
    }
    unsigned char sum[16];
    memcpy(&sum[8], m_socket->read(8).constData(), 8);
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

    m_socket->write("HTTP/1.1 101 Switching Protocols\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: roomcontrol\r\n");
    m_socket->write("sec-WebSocket-Origin: "+origin+"\r\n");
    QByteArray location = m_header.value("Host");
    if (location.isEmpty()) location = m_socket->localAddress().toString().toAscii()+":"+QByteArray::number(m_socket->localPort());
    m_socket->write("Sec-WebSocket-Location: wss://"+location+"/"+QFileInfo(m_requestedfile).fileName().toLatin1()+"\r\n");
    writeDefaultHeaders();
    m_socket->write("\r\n"+hash.result());
    m_socket->flush();
}

bool ClientConnection::readHttp(const QByteArray& line) {
    if (line.endsWith("HTTP/1.1")) { // header start line
        // request type
        m_requestType = None;
        if (line.startsWith("GET")) m_requestType = Get;
        else if (line.startsWith("HEAD")) m_requestType = Head;
        else if (line.startsWith("POST")) m_requestType = Post;
        // requested filename
        int i = line.indexOf(' ');
        QByteArray requestedfile = line.mid(i+2,line.length()-10-i).trimmed();
        i = requestedfile.indexOf('?');
        if (i!=-1) {
            QByteArray parameters = requestedfile.mid(i+1);
            requestedfile.truncate(i);
            i = 0;
            while (i!=-1) {
                i = parameters.indexOf('=', i);
                if (i==-1) break;
                QByteArray key = parameters.mid(0,i);
                i = parameters.indexOf('&', i);
                if (i==-1) break;
                QByteArray value = parameters.mid(0,i);
                m_fileparameters[key] = value;
            }
		}
		
		if (requestedfile=="")
			m_requestedfile = wwwFile(QLatin1String("index.html"));
		else {
			m_requestedfile = QUrl::fromPercentEncoding(requestedfile);
			// xml umleitung
			QFileInfo info(m_requestedfile);
			if (info.suffix() == QLatin1String("xml")) {
				QDir xmldir = pluginDir();
				xmldir.cd(QLatin1String("xml"));
				m_requestedfile = xmldir.absoluteFilePath(m_requestedfile);
			} else {
				m_requestedfile = wwwFile(m_requestedfile);
			}
		}

		// header leeren
        m_header.clear();
        m_inHeader = true;
    } else if (line.size() && m_inHeader) { // header
        int i = line.indexOf(':');
        if (i==-1) {
            qWarning() << "Client invalid header:"<<m_socket->peerAddress().toString()<<line;
            emit removeConnection(this);
            return false;
        }
        m_header.insert(line.mid(0,i).trimmed(),line.mid(i+1).trimmed());
    } else if (line.isEmpty() && m_inHeader) { // header end line, parse header data
        m_inHeader = false;
        // websocket request
        if (m_header.value("Upgrade").toLower() == "websocket" && m_header.value("Connection") == "Upgrade") {
            if (m_header.value("Sec-WebSocket-Protocol") != "roomcontrol") {
                qWarning()<<"Websocket: Wrong subprotocol";
                emit removeConnection(this);
                return false;
            }
            // auth header
            if (m_header.contains("sessionid")) {
                Session* session = SessionController::instance()->getSession(QString::fromAscii(m_header["sessionid"]));
                if (session) {
                    sessionid = session->sessionid();
                    session->resetSessionTimer();
                    m_authok = true;
                }
            }
            if (m_header.contains("Sec-WebSocket-Key")) { //Version 4+
                generateWebsocketResponseV04();
            } else if (m_header.contains("Sec-WebSocket-Key1") && m_header.contains("Sec-WebSocket-Key2") && m_socket->bytesAvailable()>=8) { //Version 0-3
                generateWebsocketResponseV00();
            } else {
                qWarning()<<"Websocket: Draft Version not recognized. Server supports draft-ietf-hybi-thewebsocketprotocol 00-06";
                emit removeConnection(this);
                return false;
            }
            m_isWebsocket = true;
            emit upgradedConnection();
        } else
            // file request
        {
            generateFileResponse();
        }
    } else {
        qWarning() << "unknown data" << line;
    }
    return true;
}

void ClientConnection::writeDefaultHeaders() {
    m_socket->write("Server: roomcontrolserver\r\nDate: " + QDateTime::currentDateTime().toString(QLatin1String("ddd, d MMMM yyyy hh:mm:ss")).toAscii() + " GMT\r\n" +
                    "Content-Language: de\r\n");
}
