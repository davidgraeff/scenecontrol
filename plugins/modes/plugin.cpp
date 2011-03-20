/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QDebug>
#include <QtPlugin>

#include "plugin.h"

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() {
}

plugin::~plugin() {

}

void plugin::initialize() {
}

void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
	PluginHelper::setSetting(name, value, init);
}

void plugin::execute(const QVariantMap& data) {
	if (IS_ID("changemode")) {
		m_mode = DATA("mode");
		QSet<QString> uids = m_mode_events[m_mode];
		foreach(QString uid, uids) {
			m_server->event_triggered(uid);
		}
		modeChanged(m_mode);
	}
}

bool plugin::condition(const QVariantMap& data)  {
	if (IS_ID("modecondition")) {
		return (m_mode == DATA("mode"));
	}
	return false;
}

void plugin::event_changed(const QVariantMap& data) {
	// entfernen
	const QString uid = DATA("uid");
	QMap<QString, QSet<QString> >::iterator it = m_mode_events.begin();
	for(;it!=m_mode_events.end();++it) {
		it->remove(uid);
	}
	// hinzufügen
	m_mode_events[DATA("mode")].insert(uid);
}

QMap<QString, QVariantMap> plugin::properties() {
	QMap<QString, QVariantMap> l;
	return l;
}

void plugin::modeChanged(const QString& mode) {
	PROPERTY("modechanged");
	data[QLatin1String("mode")] = mode;
	m_server->property_changed(data);
	
	foreach (QString uid, m_mode_events[mode]) {
		m_server->event_triggered(uid);
	}
}
