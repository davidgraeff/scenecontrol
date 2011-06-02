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

#include "paths.h"
#include "config.h"
#include "session.h"
#include "websocket.h"

SessionExtension::SessionExtension(QObject* parent) : QObject(parent), m_websocket(0) {
	m_dataLost = false;
    m_clearDataCacheTimer.setSingleShot(true);
    m_clearDataCacheTimer.setInterval(5000);
    connect(&m_clearDataCacheTimer, SIGNAL(timeout()),SLOT(clearDataCache()));
}

SessionExtension::~SessionExtension() {
	if (m_websocket)
		m_websocket->deleteLater();
}

void SessionExtension::dataChanged(const QByteArray& data) {
    if (m_websocket) // write via websocket
        m_websocket->writeJSON(data);
    else { // cache data up to 5 sec for a polling http request
        m_clearDataCacheTimer.start();
        m_dataCache.append(data);
    }
}

void SessionExtension::setWebsocket(WebSocket* websocket) {
	if (m_websocket) { // remove existing websocket
		delete m_websocket;
		m_websocket = 0;
	}
	m_websocket = websocket;
    connect(websocket,SIGNAL(removeWebSocket(WebSocket*)),SLOT(clearWebSocket(WebSocket*)));
	m_clearDataCacheTimer.stop();
	while (m_dataCache.size()) {
		m_websocket->writeJSON(m_dataCache.takeFirst());
	}
}

void SessionExtension::clearDataCache() {
	if (m_dataCache.size()) {
		m_dataLost = true;
		qWarning() << "Session lost cached data: #" << m_dataCache.size();
	}
    m_dataCache.clear();
	m_clearDataCacheTimer.stop();
}

void SessionExtension::clearWebSocket(WebSocket* websocket) {
	Q_ASSERT(m_websocket==websocket);
	m_websocket->deleteLater();
	m_websocket = 0;
}

QList< QByteArray > SessionExtension::getDataCache() {
	QList< QByteArray > a = m_dataCache;
	m_dataCache.clear();
	m_clearDataCacheTimer.stop();
	return a;
}
