/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once
#include <QVariantMap>
#include <QString>
#include <QSet>


// getter
#define MAP(ITEMID) data[QLatin1String(ITEMID)].toMap()
#define LIST(ITEMID) data[QLatin1String(ITEMID)].toList()
#define DATA(ITEMID) data[QLatin1String(ITEMID)].toString()
#define INTDATA(ITEMID) data[QLatin1String(ITEMID)].toInt()
#define BOOLDATA(ITEMID) data[QLatin1String(ITEMID)].toBool()
#define DOUBLEDATA(ITEMID) data[QLatin1String(ITEMID)].toDouble()

class ServiceCreation {
private:
    QVariantMap m_map;
    ServiceCreation() {}
public:
    ServiceCreation(const QVariantMap& map) : m_map(map) {}
	/**
	 * Creates a model item remove notification.
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createModelRemoveItem(const char* plugin_id, const char* id) ;
    
	/**
	 * Creates a model item change notification.
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createModelChangeItem(const char* plugin_id, const char* id) ;
    
	/**
	 * Creates a model reset notification.
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 * \param key Model key/index item name
	 */
    static ServiceCreation createModelReset(const char* plugin_id, const char* id, const char* key) ;
    
	/**
	 * Creates a notification
	 * \param plugin_id Which plugin does generate this notification (mostly PLUGIN_ID).
	 * \param id Notification id. Must be the same as documented in the plugin xml file.
	 */
    static ServiceCreation createNotification(const char* plugin_id, const char* id) ;

    static ServiceCreation createRemoveByUidCmd(const QString& uid, const QString& type) ;

    static ServiceCreation createExecuteByUidCmd(const QString& uid) ;
    
	/**
	 * Creates an execute cmd. Will only be propagated to the destination plugin and executed if all necessary data is set.
	 * \param plugin_id The destination plugin that implements the wanted functionality.
	 * \param id The id for this action within the destination plugin
	 */
    static ServiceCreation createExecuteByDataCmd(const char* plugin_id, const char* id) ;
    
    void setData(const char* index, const QVariant& data) ;

    QVariantMap getData() ;
};

#ifndef PLUGIN_ID 
	#define PLUGIN_ID "fake_from_pluginservicehelper.cpp"
#endif
#include "abstractserver.h"
#include "abstractplugin.h"
/**
 * Example:
 * plugin.h: EventMap<int> m_events;
 * plugin.cpp (constructor)  : EventMap<int>(key_fieldname)
 * plugin.cpp (register_event)  : m_events.add(data, collectionuid);
 * plugin.cpp (unregister_event): m_events.remove(data, collectionuid);
 * plugin.cpp (event trigger)   : m_events.getUids(key);
 */
template <class T>
class EventMap : private QMap<T, QMap<QString, QVariantMap > >
{
public:
	EventMap(const QString& fieldname) ;
	void add(const QVariantMap& data, const QString& collectionuid) ;
	void remove(const QVariantMap& data, const QString& collectionuid) ;
	QList<QVariantMap> data(T key);
	void triggerEvent(T key, AbstractServer* server) ;
private:
	QString m_fieldname;
};

template <class T>
EventMap<T>::EventMap ( const QString& fieldname ) : m_fieldname ( fieldname ) { }

template <class T>
void EventMap<T>::add ( const QVariantMap& data, const QString& collectionuid ) {
    T key = data.value ( m_fieldname ).value<T>();
    QMap<QString, QVariantMap > datas = QMap<T, QMap<QString, QVariantMap > >::take ( key );
    datas.insert ( ServiceID::id ( data ), ServiceID::newDataWithCollectionUid ( data, collectionuid ) );
    QMap<T, QMap<QString, QVariantMap > >::insert ( key, datas );
}

template <class T>
void EventMap<T>::remove ( const QVariantMap& data, const QString& collectionuid ) {
	Q_UNUSED(collectionuid);
    T key = data.value ( m_fieldname ).value<T>();
    QMap<QString, QVariantMap > datas = QMap<T, QMap<QString, QVariantMap > >::take ( key );
    datas.remove ( ServiceID::id ( data ) );
    if ( datas.size() )
        QMap<T, QMap<QString, QVariantMap > >::insert ( key, datas );
}

template <class T>
QList<QVariantMap> EventMap<T>::data(T key) {
	return QMap<T, QMap<QString, QVariantMap > >::value ( key ).values();
}

template <class T>
void EventMap<T>::triggerEvent ( T key, AbstractServer* server ) {
    QMap<QString, QVariantMap > datasMaps = QMap<T, QMap<QString, QVariantMap > >::value ( key );
    QList<QVariantMap> datas = datasMaps.values();
    foreach ( QVariantMap data, datas ) {
		server->event_triggered(ServiceID::id(data), ServiceID::collectionid ( data ));
    }
}