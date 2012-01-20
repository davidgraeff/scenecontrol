#pragma once

#ifndef PLUGIN_ID
#define PLUGIN_ID "fake_from_pluginservicehelper.cpp"
#endif

#include "pluginservicehelper.h"

#include <QVariantMap>
#include <QString>
#include <QSet>

/**
 * Example:
 * plugin.h: EventMap<int> m_events;
 * plugin.cpp (constructor)  : EventMap<int>(key_fieldname)
 * plugin.cpp (register_event)  : m_events.add(data, collectionuid);
 * plugin.cpp (unregister_event): m_events.remove(data);
 * plugin.cpp (event trigger)   : m_events.getUids(key);
 */
template <class T, char* MEMBER>
class EventMap : private QMap<T, QMap<QString, QVariantMap > >
{
public:
    EventMap() ;
    void add(const QVariantMap& data, const QString& collectionuid) ;
    QVariantMap remove(const QString& eventid) ;
    QList<QVariantMap> data(T key);
private:
    QString m_fieldname;
};

template <class T, char* MEMBER>
EventMap<T,MEMBER>::EventMap () : m_fieldname ( QString::fromAscii(MEMBER) ) { }

template <class T, char* MEMBER>
void EventMap<T,MEMBER>::add ( const QVariantMap& data, const QString& collectionuid ) {
    T key = data.value ( m_fieldname ).value<T>();
    QMap<QString, QVariantMap > datas = QMap<T, QMap<QString, QVariantMap > >::take ( key );
    QVariantMap modifieddata = data;
    modifieddata[QLatin1String("collection_")] = collectionuid;
    datas.insert ( ServiceData::id ( data ), modifieddata );
    QMap<T, QMap<QString, QVariantMap > >::insert ( key, datas );
}

template <class T, char* MEMBER>
QVariantMap EventMap<T,MEMBER>::remove ( const QString& eventid) {
    QMutableMapIterator<T, QMap<QString, QVariantMap > > outerMapIt(*this);
    while (outerMapIt.hasNext()) {
        outerMapIt.next();
        if (outerMapIt.value().contains(eventid)) {
	  const QVariantMap data = outerMapIt.value().value(eventid);
            outerMapIt.value().remove(eventid);
            if (!outerMapIt.value().size()) {
                outerMapIt.remove();
            }
            return data;
        }
    }
    return QVariantMap();
}

template <class T, char* MEMBER>
QList<QVariantMap> EventMap<T,MEMBER>::data(T key) {
    return QMap<T, QMap<QString, QVariantMap > >::value ( key ).values();
}
//     foreach ( QVariantMap& data, datas ) {
//         server->pluginEventTriggered(ServiceData::id(data), ServiceData::collectionid ( data ));
//     }

