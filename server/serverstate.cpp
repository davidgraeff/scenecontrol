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

*/

#include "serverstate.h"
#include <QDebug>
#include <QCoreApplication>

QList< QVariantMap > ServerState::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    return l;
}

bool ServerState::condition(const QVariantMap& data)  {
    Q_UNUSED(data);
	return false;
}

void ServerState::event_changed(const QVariantMap& data)  {
	if (IS_ID("serverevent")) {
		// entfernen
		const QString uid = UNIQUEID();
		QMutableMapIterator<int, QSet<QString> > it(m_state_events);
		while (it.hasNext()) {
			it.next();
			it.value().remove(uid);
			if (it.value().isEmpty())
				it.remove();
		}
		// hinzufügen
		m_state_events[INTDATA("state")].insert(uid);
	}
}

void ServerState::execute(const QVariantMap& data)  {
    if (IS_ID("servercmd")) {
        switch (INTDATA("state")) {
        case 0:
            QCoreApplication::exit(0);
            break;
        default:
            break;
        }
    }
}

void ServerState::initialize() {
    PROPERTY("serverstate");
    data[QLatin1String("state")] = 1;
    m_server->property_changed(data);
	
	foreach (QString uid, m_state_events.value(1)) {
		m_server->event_triggered(uid);
	}
}

void ServerState::clear() {
    PROPERTY("serverstate");
    data[QLatin1String("state")] = 0;
    m_server->property_changed(data);
	
	foreach (QString uid, m_state_events.value(0)) {
		m_server->event_triggered(uid);
	}
}
