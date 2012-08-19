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
#include <QUdpSocket>
#include <QHostAddress>
#include "plugin.h"
#include <qfile.h>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& instanceid) : AbstractPlugin(instanceid) {}

plugin::~plugin() {}

void plugin::wol ( const QString& mac) {
    QList<QByteArray> parts = mac.toAscii().split ( ':' );
    if ( parts.size() !=6 ) return;
    QByteArray macdecoded;
    for ( int i=0;i<6;++i )
        macdecoded.append ( QByteArray::fromHex ( parts[i] ) );

    // 6 mal FF
    const unsigned char header[] = {255,255,255,255,255,255};
    QByteArray bytes ( (const char*)header );
    // 16 mal mac
    for ( int i=0;i<16;++i )
        bytes.append ( macdecoded );

    QUdpSocket socket;
    socket.writeDatagram ( bytes,QHostAddress::Broadcast,9 );
}

void plugin::requestProperties(int sessionid) {
    changeProperty(ServiceData::createModelReset("wol.arpcache", "mac").getData(), sessionid);

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
            changeProperty(sc.getData(), sessionid);
        }
        file.close();
    }
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
  Q_UNUSED(plugin_id);
  Q_UNUSED(data);
}
