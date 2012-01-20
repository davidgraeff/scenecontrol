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
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() {
}

plugin::~plugin() {

}

bool plugin::condition(const QVariantMap& data)  {
    Q_UNUSED(sessionid);
    if (ServiceData::isMethod(data,"modecondition")) {
        return (m_mode == DATA("mode"));
    }
    return false;
}

void plugin::eventmode ( const QString& eventid, const QString& mode, const QString& collectionuid) {
  m_collectionsOnMode.insertMulti(mode, QPair<QString,QString>(eventid, collectionuid>));
}

void plugin::unregister_event ( const QString& eventid) {
    Q_UNUSED(sessionid);
    QMutableMapIterator< QString, QPair<QString, QString> > i = m_collectionsOnMode;
    while (i.hasNext()) {
      i.next();
      if (i.value().first == eventid)
	i.remove();
    }
}

void plugin::requestProperties(int sessionid) {
    Q_UNUSED(sessionid);
    ServiceData sc = ServiceData::createNotification(PLUGIN_ID, "mode");
    sc.setData("mode", m_mode);
    changeProperty(sc.getData());
}

void plugin::modeChange(const QString& mode) {
    m_mode = mode;
    ServiceData sc = ServiceData::createNotification(PLUGIN_ID, "mode");
    sc.setData("mode", mode);
    changeProperty(sc.getData());

    QList< QPair<QString, QString> > list = m_collectionsOnMode.values(mode);
    while (const QPair<QString, QString>& data, list) {
        eventTriggered(ServiceData::id(data), ServiceData::collectionid ( data ).toAscii());
    }
}
