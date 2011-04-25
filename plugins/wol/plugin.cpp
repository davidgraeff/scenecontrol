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
#include <QUdpSocket>
#include <QHostAddress>
#include "plugin.h"

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() {
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
	if (ServiceID::isId(data,"wol")) {
		QStringList parts = data["mac"].toString().split(QLatin1Char(':'));
		if (parts.size()!=6) return;
		QByteArray mac;
		for (int i=0;i<6;++i)
			mac.append(QByteArray::fromHex(parts[i].toAscii()));

		// 6 mal FF
		const char header[] = {255,255,255,255,255,255};
		QByteArray bytes(header);
		// 16 mal mac
		for (int i=0;i<16;++i)
			bytes.append(mac);

		QUdpSocket socket;
		socket.writeDatagram(bytes,QHostAddress::Broadcast,9);
	}
}

bool plugin::condition(const QVariantMap& data, const QString& sessionid)  {
	Q_UNUSED(data);
	return false;
}

void plugin::event_changed(const QVariantMap& data, const QString& sessionid) {
	Q_UNUSED(data);
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
Q_UNUSED(sessionid);
	QList<QVariantMap> l;
	return l;
}
