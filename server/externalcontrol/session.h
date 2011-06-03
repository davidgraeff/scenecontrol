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

#include <QByteArray>
#include <QTimer>

class WebSocket;
class SessionExtension : public QObject
{
    Q_OBJECT
public:
    SessionExtension(const QString& sessionid, QObject* parent) ;
    ~SessionExtension() ;
    void dataChanged(const QByteArray& data);
	void setWebsocket(WebSocket* websocket);
	QList<QByteArray> getDataCache();
	QString sessionid();
	void finishAfterTimeout();
private:
	QString m_sessionid;
	WebSocket* m_websocket;
    QTimer m_clearDataCacheTimer;
	QList<QByteArray> m_dataCache;
	bool m_dataLost;
	bool m_closeAfterTimeout;
private Q_SLOTS:
    void clearDataCache();
    void clearWebSocket(WebSocket*);
Q_SIGNALS:
	void removeSession(SessionExtension*);
};
