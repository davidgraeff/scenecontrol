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


ClientConnection::ClientConnection(QSslSocket* s) : m_socket(s) {
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

    m_oldhandshake = false;
    m_inHeader = false;
}
ClientConnection::~ClientConnection() {
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
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        // in header
        if (m_oldhandshake) {
			m_oldhandshake = false;
            if (line.size()!=8) {
                qWarning()<<"Websocket: Protocol < 4 Key Handshake failed. Body != 8 Byte";
                emit removeConnection(this);
                return;
            }
            QByteArray s1 = m_header.value("Sec-WebSocket-Key1");
            QByteArray s2 = m_header.value("Sec-WebSocket-Key2");
            uint32_t number1 = 0, number2 = 0;
            int count1 = 0, count2 = 0;
            for (int i=0;i<s1.count();++i) {
                if (s1[i] == ' ') count1++;
                if (QChar::fromAscii(s1[i]).isDigit()) number1 += i*10+(s1[i]-'0');
            }
            for (int i=0;i<s2.count();++i) {
                if (s2[i] == ' ') count2++;
                if (QChar::fromAscii(s2[i]).isDigit()) number2 += i*10+(s2[i]-'0');
            }
            if (count1==0 || count2==0) {
                qWarning()<<"Websocket: Protocol < 4 Key Handshake failed. Wrong spaces.";
                emit removeConnection(this);
                return;
            }
            number1 /= count1;
            number2 /= count2;
            QCryptographicHash hash(QCryptographicHash::Md5);
            hash.addData((char*)&number1,sizeof(uint32_t));
            hash.addData((char*)&number2,sizeof(uint32_t));
            hash.addData(line);
            qDebug() << "hash: "+hash.result().toHex();
            m_socket->write("\n\n"+hash.result());
            m_socket->flush();
        } else if (line.startsWith("GET") && line.endsWith("HTTP/1.1")) {
            m_requestedfile = line.mid(5,line.length()-9-5).trimmed();
            if (m_requestedfile=="") m_requestedfile = "index.html";
            // header leeren
            m_header.clear();
            m_inHeader = true;
        } else if (line.size() && m_inHeader) { // header
            const QList<QByteArray> key_value = line.split(':');
            if (key_value.size()!=2) continue;
            m_header.insert(key_value[0].trimmed(),key_value[1].trimmed());
        } else if (line.isEmpty() && m_inHeader) {
            // websocket

            if (m_header.value("Upgrade").toLower() == "websocket" && m_header.value("Connection") == "Upgrade") {
                qDebug()<<"start websocket...";
                if (m_header.value("Sec-WebSocket-Protocol") != "roomcontrol") {
                    qWarning()<<"Websocket: Wrong subprotocol";
                    emit removeConnection(this);
                    return;
                }
                // auth header
                if (m_header.contains("sessionid")) {
                    Session* session = SessionController::instance()->getSession(QString::fromAscii(m_header["sessionid"]));
                    if (session) {
                        session->resetSessionTimer();
                        m_authok = true;
                    }
                }
                m_socket->write("HTTP/1.1 101 Switching Protocols\nUpgrade: websocket\nConnection: Upgrade\nSec-WebSocket-Protocol: roomcontrol\n");
                if (m_header.contains("Sec-WebSocket-Key")) { //Version 4+
                    QCryptographicHash hash(QCryptographicHash::Sha1);
                    hash.addData(m_header.value("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
                    qDebug() << "hash: "+hash.result().toBase64();
                    m_socket->write("Sec-WebSocket-Accept: "+hash.result().toBase64()+"\n\n");
                } else if (m_header.contains("Sec-WebSocket-Key1") && m_header.contains("Sec-WebSocket-Key2")) { //Version 0-3
                    m_oldhandshake = true;
                }
                m_socket->flush();
                m_isServerEventConnection = true;
                qDebug() << "start server socket";
            } else {
                QFile www(wwwFile(QUrl::fromPercentEncoding(m_requestedfile)));
                QFileInfo wwwinfo(www);
                if (www.exists()) {
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
                    }
                    www.open(QIODevice::ReadOnly);
                    generateResponse(200, www.readAll(), type);
                    www.close();
                } else {
                    generateResponse(404);
                }
            }
            m_inHeader = false;
        } else if (line.startsWith("{") && line.endsWith("}")) { // json
qDebug() << "json receive" << line;
            bool ok = true;
            const QVariantMap data = QJson::Parser().parse (line, &ok).toMap();
            if (!ok) continue;
qDebug() << "json receive" << data;
            if (IS_ID("sessionlogin")) { // login have to happen here
                sessionid = SessionController::instance()->addSession(DATA("user"),DATA("pwd"));
            } else if (IS_ID("sessionidle")) {
                Session* session = SessionController::instance()->getSession(sessionid);
                Q_ASSERT(session);
                session->resetSessionTimer();
            }
            else emit dataReceived(data, sessionid);
        } else {
            qDebug() << "unknown data" << line;
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
//         QByteArray data;
//         data.append("{\"type\" : \"serverinfo\", \"version\" : \""ROOM_NETWORK_APIVERSION"\", \"auth\" : \"required\", \"auth_timeout\" : \"");
//         data.append(QByteArray::number(ROOM_NETWORK_AUTHTIMEOUT));
//         data.append("\", \"plugins\" : \"");
//         data.append("\"}");
//         socket->write ( data );
    //qDebug() << "new connection, waiting for authentification";

}

void ClientConnection::sessionTimeout() {
    m_authok=false;
}

void ClientConnection::sessionFailed() {
    m_authok=false;
}

bool ClientConnection::isAuthentificatedServerEventConnection() {
    return m_authok && m_isServerEventConnection;
}

void ClientConnection::writeJSON(const QByteArray& data) {
    if (!m_isServerEventConnection) return;
    m_socket->write(data+"\n");
}

void ClientConnection::generateResponse(int httpcode, const QByteArray& data, const QByteArray& contenttype) {
    switch (httpcode) {
    case 404:
        m_socket->write("HTTP/1.1 404 Not Found\n");
        break;
    case 200:
        m_socket->write("HTTP/1.1 200 OK\n");
        break;
    default:
        m_socket->write("HTTP/1.1 404 Not Found\n");
        break;
    }
    m_socket->write("Server: roomcontrolserver\nDate: " + QDateTime::currentDateTime().toString(QLatin1String("ddd, d MMMM yyyy hh:mm:ss")).toAscii() + " GMT\n" +
                    "Content-Language: de\ncharset=utf-8\n");
    m_socket->write("Content-Type: " + contenttype + "\n");
    m_socket->write("Content-Length:"+QByteArray::number(data.size())+"\n\n");
    if (data.size()) m_socket->write(data);
    m_socket->flush();
}

void ClientConnection::timeout() {
    emit removeConnection(this);
}
