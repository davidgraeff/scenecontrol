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

plugin::plugin() : m_card_events(QLatin1String("cardid")) {
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

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED ( data );
	Q_UNUSED(sessionid);
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( data );
	Q_UNUSED(sessionid);
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) { 
	Q_UNUSED(sessionid);
    if (ServiceID::isMethod(data,"cardevent")) {
		m_card_events.add(data, collectionuid);
    }
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) { 
	Q_UNUSED(sessionid);
	m_card_events.remove( eventid );
}

QList<QVariantMap> plugin::properties(int sessionid) {
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
    m_serverPropertyController->pluginPropertyChanged(sc.getData());

	m_card_events.triggerEvent(atr, m_server);
}
