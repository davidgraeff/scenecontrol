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

#pragma once

#include <QtNetwork/QSslSocket>
#include <QByteArray>
#include <QTimer>

class HttpRequest : public QObject
{
    Q_OBJECT
	friend class WebSocket;
	friend class HttpServer;
    friend class HttpResponseFile;
	friend class HttpResponseSendJson;
	friend class HttpResponseReceiveJson;
public:
    HttpRequest(QSslSocket* s, QObject* parent) ;
    ~HttpRequest() ;
	QSslSocket* takeOverSocket();
	enum HttpRequestType {
		RequestTypeNone,
		RequestTypeFile,
		RequestTypeWebsocket,
		RequestTypeSendJSon,
		RequestTypePollJSon
	} httprequestType;
private:
    QString m_sessionid;
    bool m_inHeader;
    QMap<QByteArray,QByteArray> m_header;
    QString m_requestedfile;
    QMap<QByteArray, QByteArray> m_fileparameters;
    enum enumRequestType {
        None,
        Get,
        Post,
        Head
    } m_requestType;
    //network
    QSslSocket* m_socket;
    QTimer m_timeout;
	QTimer m_waitForData;
    bool readHttpRequest(const QByteArray& line);
    bool readHttpHeader(const QByteArray& line);
    void parseHeaders();
private Q_SLOTS:
    /**
     * Close this connection after 5min of inactivity
     */
    void timeout();
	void timeoutWaitForData();
	/**
	 * Socket closed
	 */
    void disconnected();
	/**
	 * Socket has new data
	 */
    void readyRead ();
    void peerVerifyError(QSslError);
    void sslErrors(QList<QSslError>);
Q_SIGNALS:
	/**
	 * Socket disconnected or some error occured while parsing headers
	 */
    void removeConnection(HttpRequest*);
	/**
	 * Headers successfully parsed
	 */
	void headerParsed(HttpRequest*);
};
