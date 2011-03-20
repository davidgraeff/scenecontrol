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

void plugin::initialize(){
    m_thread->start();
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginHelper::setSetting ( name, value, init );
}

void plugin::execute ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

bool plugin::condition ( const QVariantMap& data )  {
    Q_UNUSED ( data );
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
	// entfernen
	const QString uid = DATA("uid");
	QMap<QString, QSet<QString> >::iterator it = m_card_events.begin();
	for(;it!=m_card_events.end();++it) {
		it->remove(uid);
	}
	// hinzufügen
	m_card_events[DATA("cardid")].insert(uid);
}

QMap<QString, QVariantMap> plugin::properties() {
    QMap<QString, QVariantMap> l;
    return l;
}

void plugin::cardDetected ( const QString& atr, int state ) {
    PROPERTY("cardevent");
	data[QLatin1String("cardid")] = atr;
	data[QLatin1String("state")] = state;
    m_server->property_changed(data);
	
	foreach (QString uid, m_card_events[atr]) {
		m_server->event_triggered(uid);
	}
}
