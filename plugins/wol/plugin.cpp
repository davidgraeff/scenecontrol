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
#include <qfileinfo.h>



plugin::plugin() {
}

plugin::~plugin() {

}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {Q_UNUSED(data);}

void plugin::execute ( const QVariantMap& data) {
	Q_UNUSED(sessionid);
    if ( ServiceData::isMethod ( data,"wol" ) ) {
        QStringList parts = data["mac"].toString().split ( QLatin1Char ( ':' ) );
        if ( parts.size() !=6 ) return;
        QByteArray mac;
        for ( int i=0;i<6;++i )
            mac.append ( QByteArray::fromHex ( parts[i].toAscii() ) );

        // 6 mal FF
        const char header[] = {255,255,255,255,255,255};
        QByteArray bytes ( header );
        // 16 mal mac
        for ( int i=0;i<16;++i )
            bytes.append ( mac );

        QUdpSocket socket;
        socket.writeDatagram ( bytes,QHostAddress::Broadcast,9 );
    }
}

bool plugin::condition ( const QVariantMap& data)  {
    Q_UNUSED ( data );
	Q_UNUSED(sessionid);
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid) { 
	Q_UNUSED(sessionid);
    Q_UNUSED ( data );
	Q_UNUSED(collectionuid);
}

void plugin::unregister_event ( const QString& eventid) { 
	Q_UNUSED(sessionid);
	Q_UNUSED(eventid);
}

QList<QVariantMap> plugin::properties (  ) {
    Q_UNUSED ( sessionid );

	changeProperty(ServiceData::createModelReset("wol.arpcache", "mac").getData());
	
    QFile file(QLatin1String("/proc/net/arp"));
    if (file.exists() && file.open(QFile::ReadOnly)) {
		file.readLine(); // ignore first line
		while (file.canReadLine()) {
			QByteArray line = file.readLine();
			// get ip
			int c = line.indexOf(' ', 0);
			QByteArray ip = line.mid(0, c);
			while (line.size()>c && line[c] == ' ') ++c;
			// jump over HW type
			c = line.indexOf(' ', c);
			while (line.size()>c && line[c] == ' ') ++c;
			// jump over flags
			c = line.indexOf(' ', c);
			while (line.size()>c && line[c] == ' ') ++c;
			// get mac
			QByteArray mac = line.mid(c, line.indexOf(' ', c) - c);
			
			ServiceData sc = ServiceData::createModelChangeItem("wol.arpcache");
            sc.setData("mac", mac);
            sc.setData("ip", ip);
            changeProperty(sc.getData());
		}
		file.close();
	}
    return l;
}
