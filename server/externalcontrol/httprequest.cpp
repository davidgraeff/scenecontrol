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
#include <QUrl>
#include <QDebug>
#include <QHostAddress>

#include "config.h"
#include "paths.h"
#include "httprequest.h"
#include "sessioncontroller.h"

#define __FUNCTION__ __FUNCTION__

HttpRequest::HttpRequest(QSslSocket* s, QObject* parent) : QObject(parent), m_socket(s) {
    connect(m_socket, SIGNAL ( readyRead() ), SLOT ( readyRead() ) );
    connect(m_socket, SIGNAL(peerVerifyError(const QSslError &)),
            this, SLOT(peerVerifyError (const QSslError &)));
    connect(m_socket, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(sslErrors(const QList<QSslError> &)));

    connect(m_socket,SIGNAL(disconnected()),SLOT(disconnected()));
    connect(&m_timeout, SIGNAL(timeout()), SLOT(disconnected()));

    httprequestType = RequestTypeNone;
    m_inHeader = false;

	connect(&m_waitForData, SIGNAL(timeout()), SLOT(timeoutWaitForData()));
	m_waitForData.setSingleShot(true);
	m_waitForData.setInterval(1000);
	
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(5*60*1000); // keep inactive sockets 5 minutes
    m_timeout.start();
}

HttpRequest::~HttpRequest() {
    if (m_socket) {
        m_socket->blockSignals(true);
        m_socket->close();
        delete m_socket;
    }
}

void HttpRequest::readyRead()
{
    // disconnect socket after timeout
    m_timeout.stop();
    m_timeout.start();

    if (m_socket->bytesAvailable() > 1024*2000) {
        m_socket->readAll();
        qWarning() << "receive incomplete";
        return;
    }

    while (m_socket->canReadLine()) {
        const QByteArray line = m_socket->readLine().trimmed();
        bool s = false;
        if (line.endsWith("HTTP/1.1") || line.endsWith("HTTP/1.0")) { // header start line
			httprequestType = RequestTypeNone;
            s = readHttpRequest(line);
        } else if (line.size() && m_inHeader) { // header
            s = readHttpHeader(line);
        } else if (line.isEmpty() && m_inHeader) { // header end line, parse header data
            m_inHeader = false;
            // auth header
            if (m_sessionid.isEmpty() && m_header.contains("Sessionid")) {
                Session* session = SessionController::instance()->getSession(QString::fromAscii(m_header["Sessionid"]));
                if (session) {
                    m_sessionid = session->sessionid();
                    session->resetSessionTimer();
                }
            }
            // websocket request
            if (m_header.value("Upgrade").toLower() == "websocket" && m_header.value("Connection") == "Upgrade") {
                httprequestType = RequestTypeWebsocket;
            }
            // wait for data, if Content-Length given. Cases: Timeout, socket close; Data received; New http request
            if (m_header.contains("Content-Length") &&  m_socket->bytesAvailable() < m_header.value("Content-Length").toInt()) {
				m_waitForData.start();
				return;
			} else 
				emit headerParsed(this);
            break; // do not read further after http header ended
        } else {
            qWarning() << "unknown data" << line;
        }
        if (!s)
            emit removeConnection(this);
    }
}

void HttpRequest::sslErrors(QList< QSslError > errors) {
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

void HttpRequest::peerVerifyError(QSslError error) {
    switch (error.error()) {
    default:
        qWarning()<<__FUNCTION__ << m_socket->peerAddress() << error.errorString();
        break;
    case QSslError::SelfSignedCertificate:
        break;
    }
}

void HttpRequest::disconnected()
{
    emit removeConnection(this);
}

void HttpRequest::timeout() {
    emit removeConnection(this);
}

bool HttpRequest::readHttpRequest(const QByteArray& line) {
    // request type
    m_requestType = None;
	m_sessionid = QString();
    if (line.startsWith("GET")) m_requestType = Get;
    else if (line.startsWith("HEAD")) m_requestType = Head;
    else if (line.startsWith("POST")) m_requestType = Post;
    else
        return false;
	
    // requested filename
	httprequestType = RequestTypeFile;
	
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
        } else if (info.fileName()==QLatin1String("send.json")) {
            httprequestType = RequestTypeSendJSon;
        } else if (info.fileName()==QLatin1String("poll.json")) {
            httprequestType = RequestTypePollJSon;
        } else {
            m_requestedfile = wwwFile(m_requestedfile);
        }
    }

    // header leeren
    m_header.clear();
    m_inHeader = true;
    return true;
}

bool HttpRequest::readHttpHeader(const QByteArray& line) {
    int i = line.indexOf(':');
    if (i==-1) {
        qWarning() << "Client invalid header:"<<m_socket->peerAddress().toString()<<line;
        return false;
    }
    m_header.insert(line.mid(0,i).trimmed(),line.mid(i+1).trimmed());
	return true;
}

QSslSocket* HttpRequest::takeOverSocket() {
    if (!m_socket) return 0;

    QSslSocket* s = m_socket;
    disconnect(m_socket, SIGNAL ( readyRead() ), this, SLOT ( readyRead() ) );
    disconnect(m_socket, SIGNAL(peerVerifyError(const QSslError &)),
               this, SLOT(peerVerifyError (const QSslError &)));
    disconnect(m_socket, SIGNAL(sslErrors(const QList<QSslError> &)),
               this, SLOT(sslErrors(const QList<QSslError> &)));
    disconnect(m_socket,SIGNAL(disconnected()),this, SLOT(disconnected()));
	m_socket = 0;
    return s;
}

void HttpRequest::timeoutWaitForData() {
    emit headerParsed(this);
}
