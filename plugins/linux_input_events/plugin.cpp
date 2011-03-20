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
#include "configplugin.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() : m_repeat(0), m_repeatInit(0) {
    connect(&m_repeattimer,SIGNAL(timeout()),SLOT(repeattrigger()));
    m_repeattimer.setSingleShot(true);
    _config ( this );
}

plugin::~plugin() {

}

void plugin::initialize() {
    m_server->register_listener ( QLatin1String ( "selected_input_device" ) );
}


void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginHelper::setSetting ( name, value, init );
	if (name == QLatin1String("repeat")) m_repeat = value.toInt();
	else if (name == QLatin1String("repeat_init")) m_repeatInit = value.toInt();
}

void plugin::execute ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

bool plugin::condition ( const QVariantMap& data )  {
    Q_UNUSED ( data );
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    if ( IS_ID ( "inputevent" ) ) {
        // entfernen
        const QString inputdevice = DATA ( "inputdevice" );
        const QString key = inputdevice+DATA ( "kernelkeyname" );
        const QString uid = DATA ( "uid" );
        QMutableMapIterator<QString, eventstruct > it ( m_key_events );
        while ( it.hasNext() ) {
            it.value().uids.remove ( uid );
            if ( it.value().uids.isEmpty() )
                it.remove();
        }
        // hinzufügen
        m_key_events[key].uids.insert ( uid );

        // stop listening to device
        if ( m_eventdevices.contains ( uid ) ) {
            m_eventdevices.remove ( uid );
            stoplistenToDevice ( inputdevice );
        }
        // start listening to the device
        listenToDevice ( inputdevice );
    }
}

void plugin::otherPropertyChanged ( const QVariantMap& data, const QString& sessionid ) {
    if ( IS_ID ( "selected_input_device" ) && m_sessions.contains ( sessionid ) ) {
        listenToDevice ( DATA ( "inputdevice" ), sessionid );
    }
}

void plugin::session_change ( const QString& id, bool running ) {
    PluginHelper::session_change ( id, running );
    if ( running ) return;
    // stop listening to device
    if ( m_sessiondevices.contains ( id ) ) {
        stoplistenToDevice ( m_eventdevices.value(id) );
        m_eventdevices.remove ( id );
    }
}

QMap<QString, QVariantMap> plugin::properties() {
    QMap<QString, QVariantMap> l;
    return l;
}

void plugin::eventKeyUp ( const QString& device, const QString& kernelkeyname )
{
	Q_UNUSED(device);
	Q_UNUSED(kernelkeyname);
	// repeat stop
	m_repeattimer.stop();
}

void plugin::eventKeyDown ( const QString& device, const QString& kernelkeyname ) {
	// properties
	{
		// last key property. Will be propagated to interested clients only.
		PROPERTY ( "lastinputkey" );
		data[QLatin1String ( "kernelkeyname" ) ] = kernelkeyname;
		data[QLatin1String ( "inputdevice" ) ] = device;
		
		// Propagate to all interested clients
		QMap<QString, QString>::iterator it = m_sessiondevices.begin();
		for (;it != m_sessiondevices.end();++it) {
			const QString sessionid = it.key();
			if (it.value() == device) m_server->property_changed(data, sessionid);
		}
	}
	// repeat stop
	m_repeattimer.stop();
	m_lastevent = device+kernelkeyname;
	repeattrigger(true);
}

void plugin::listenToDevice ( const QString& device, const QString& sesssionid ) {}

void plugin::stoplistenToDevice ( const QString& device, bool stopOnlyIfNotUsed ) {}

void plugin::inputDevicesChanged() {
    PROPERTY ( "inputdevice" );
    QStringList l = m_alldevices.toList();
    data[QLatin1String ( "alldevices" ) ] = l.join ( QLatin1String ( ";" ) );
    m_server->property_changed ( data );
}

void plugin::repeattrigger(bool initial_event) {
    QMap<QString, eventstruct >::iterator it = m_key_events.find(m_lastevent);
	if (it == m_key_events.end()) return;
	
	const eventstruct* event = &(*it);
	foreach (QString uid, event->uids) {
		m_server->event_triggered(uid);
	}
	
	if (!event->repeat) return;
	
	if (initial_event)
		m_repeattimer.start(initial_event?m_repeatInit:m_repeat);
}
