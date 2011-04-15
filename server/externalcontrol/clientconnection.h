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

class ServiceController;
class ClientConnection : public QObject
{
    Q_OBJECT
private:
    bool m_authok;
    bool m_isWebsocket;
    bool m_inHeader;
    QMap<QByteArray,QByteArray> m_header;
    QByteArray m_requestedfile;
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
    void generateFileResponse();
	void writeDefaultHeaders();
    void generateWebsocketResponseV04();
	void generateWebsocketResponseV00();
	bool readHttp(const QByteArray& line);
public:
    QString sessionid;

    ClientConnection(QSslSocket* s) ;
    ~ClientConnection() ;
    void sessionEstablished();
	/**
	 * Has to be websocket connection to write something
	 */
    void writeJSON(const QByteArray& data);
private Q_SLOTS:
    /**
     * Close this connection after 5min of inactivity
     */
    void timeout();

    void readyRead ();
    void disconnected();
    void peerVerifyError(QSslError);
    void sslErrors(QList<QSslError>);
Q_SIGNALS:
    void dataReceived(const QVariantMap& data, const QString& sessionid);
    void removeConnection(ClientConnection*);
	void upgradedConnection();
};
