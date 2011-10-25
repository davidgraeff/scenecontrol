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

plugin::plugin() : m_mode_events(QLatin1String("mode")) {
}

plugin::~plugin() {

}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
    PluginSettingsHelper::setSetting(name, value, init);
}

void plugin::execute(const QVariantMap& data, int sessionid) {
    Q_UNUSED(sessionid);
    if (ServiceID::isMethod(data,"changemode")) {
        m_mode = DATA("mode");
        modeChanged(m_mode);
    }
}

bool plugin::condition(const QVariantMap& data, int sessionid)  {
    Q_UNUSED(sessionid);
    if (ServiceID::isMethod(data,"modecondition")) {
        return (m_mode == DATA("mode"));
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED(collectionuid);
    if (ServiceID::isMethod(data,"modeevent")) {
        m_mode_events.add(data, collectionuid);
    }
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    m_mode_events.remove(data, collectionuid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
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
    m_server->pluginPropertyChanged(sc.getData());

    m_mode_events.triggerEvent(mode, m_server);
}
