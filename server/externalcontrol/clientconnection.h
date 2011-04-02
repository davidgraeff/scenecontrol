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

class ServiceController;
class ClientConnection : public QObject
{
    Q_OBJECT
private:
    bool m_authok;
    bool m_isServerEventConnection;
    bool m_inHeader;
    QMap<QByteArray,QByteArray> m_header;
    QByteArray m_requestedfile;
    //network
    QSslSocket* m_socket;
    void generateResponse(int httpcode, const QByteArray& data = QByteArray(), const QByteArray& contenttype = "text/html");
public:
    QString sessionid;

    ClientConnection(QSslSocket* s) ;
    ~ClientConnection() ;
    void setAuthOK();
    void setAuthNotOK();
    bool isAuthentificatedServerEventConnection();
    void writeJSON(const QByteArray& data);
private Q_SLOTS:
    void readyRead ();
    void disconnected();
    void peerVerifyError(QSslError);
    void sslErrors(QList<QSslError>);
Q_SIGNALS:
    void dataReceived(const QVariantMap& data, const QString& sessionid);
    void removeConnection(ClientConnection*);
};
