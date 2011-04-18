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
//#include "configplugin.h"
#include "controller.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_thread = new CardThread();
    connect ( m_thread,SIGNAL ( cardDetected ( QString,int ) ),SLOT ( slotcardDetected ( QString,int ) ) );
    //_config(this);
}

plugin::~plugin() {
    m_thread->abort();
    m_thread->wait();
    delete m_thread;
}

void plugin::clear() {}
void plugin::initialize() {
    m_thread->start();
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
}

void plugin::execute ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

bool plugin::condition ( const QVariantMap& data )  {
    Q_UNUSED ( data );
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    if (ServiceID::isId(data,"cardevent")) {
        // entfernen
        const QString uid = ServiceType::uniqueID(data);
        QMutableMapIterator<QString, QSet<QString> > it(m_card_events);
        while (it.hasNext()) {
            it.next();
            it.value().remove(uid);
            if (it.value().isEmpty())
                it.remove();
        }
        // hinzufügen
        m_card_events[DATA("cardid")].insert(uid);
    }
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
		l.append(ServiceCreation::createModelReset(PLUGIN_ID, "card.atr", "cardid").getData());
        ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "card.atr");
        sc.setData("cardid",m_thread->getAtr());
        sc.setData("state",m_thread->getState());
		l.append(sc.getData());
    }
    return l;
}

void plugin::slotcardDetected ( const QString& atr, int state ) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "card.atr");
    sc.setData("cardid", atr);
    sc.setData("state", state);
    m_server->property_changed(sc.getData());

    foreach (QString uid, m_card_events.value(atr)) {
        m_server->event_triggered(uid);
    }
}
