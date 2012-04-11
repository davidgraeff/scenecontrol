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
#include "plugin.h"
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

plugin::plugin(const QString& instanceid) : AbstractPlugin(instanceid) {
}

plugin::~plugin() {

}

bool plugin::isMode(const QString& mode)  {
    return (m_mode == mode);
}

void plugin::eventmode ( const QString& _id, const QString& collection_, const QString& mode) {
    m_collectionsOnMode.insertMulti(mode, QPair<QString,QString>(_id, collection_));
}

void plugin::unregister_event ( const QString& eventid) {
  const QByteArray eventid2 = eventid.toAscii();
    QMutableMapIterator< QString, QPair<QString, QString> > i = m_collectionsOnMode;
    while (i.hasNext()) {
        i.next();
        if (i.value().first == eventid2)
            i.remove();
    }
}

void plugin::requestProperties(int sessionid) {
    ServiceData sc = ServiceData::createNotification("mode");
    sc.setData("mode", m_mode);
    changeProperty(sc.getData(), sessionid);
}

void plugin::modeChange(const QString& mode) {
    m_mode = mode;
    ServiceData sc = ServiceData::createNotification("mode");
    sc.setData("mode", mode);
    changeProperty(sc.getData());

    QList< QPair<QString, QString> > list = m_collectionsOnMode.values(mode);
    for (int i=0;i<list.size(); ++i) {
        eventTriggered(list[i].first, list[i].second);
    }
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}
