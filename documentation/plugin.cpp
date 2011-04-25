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
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() {
    _config(this);
}

plugin::~plugin() {

}

void plugin::clear() {}
void plugin::initialize() {
}


void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
	PluginSettingsHelper::setSetting(name, value, init);
}

void plugin::execute(const QVariantMap& data, const QString& sessionid) {
	Q_UNUSED(sessionid);
	if (ServiceID::isId(data,"bla")) {
	}
}

bool plugin::condition(const QVariantMap& data, const QString& sessionid)  {
	Q_UNUSED(data);
	Q_UNUSED(sessionid);
	return false;
}

void plugin::event_changed(const QVariantMap& data, const QString& sessionid) {
	Q_UNUSED(data);
	Q_UNUSED(sessionid);
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
Q_UNUSED(sessionid);
	QList<QVariantMap> l;
	return l;
}
