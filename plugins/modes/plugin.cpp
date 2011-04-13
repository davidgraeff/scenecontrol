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

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
    PluginSettingsHelper::setSetting(name, value, init);
}

void plugin::execute(const QVariantMap& data) {
    if (ServiceID::isId(data,"changemode")) {
        m_mode = DATA("mode");
        modeChanged(m_mode);
    }
}

bool plugin::condition(const QVariantMap& data)  {
    if (ServiceID::isId(data,"modecondition")) {
        return (m_mode == DATA("mode"));
    }
    return false;
}

void plugin::event_changed(const QVariantMap& data) {
    if (ServiceID::isId(data,"modeevent")) {
        // entfernen
        const QString uid = ServiceType::uniqueID(data);
        QMutableMapIterator<QString, QSet<QString> > it(m_mode_events);
        while (it.hasNext()) {
            it.next();
            it.value().remove(uid);
            if (it.value().isEmpty())
                it.remove();
        }
        // hinzufügen
        m_mode_events[DATA("mode")].insert(uid);
    }
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "mode");
        sc.setData("mode", m_mode);
        l.append(sc.getData());
    }
    return l;
}

void plugin::modeChanged(const QString& mode) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "mode");
    sc.setData("mode", mode);
    m_server->property_changed(sc.getData());

    foreach (QString uid, m_mode_events.value(mode)) {
        m_server->event_triggered(uid);
    }
}
