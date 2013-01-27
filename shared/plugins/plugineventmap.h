#pragma once

#ifndef PLUGIN_ID
#define PLUGIN_ID "fake_from_pluginservicehelper.cpp"
#endif

#include "shared/jsondocuments/scenedocument.h"

#include <QVariantMap>
#include <QString>
#include <QSet>

/**
 * Example:
 * plugin.h: EventMap<int> m_events;
 * plugin.cpp (constructor)  : EventMap<int>(key_fieldname)
 * plugin.cpp (eventMethod, scene id valid)  : m_events.add(data, sceneid);
 * plugin.cpp (eventMethod, scene id empty): m_events.remove(data);
 * plugin.cpp (event trigger)   : m_events.data(key);
 */
template <class T, char* MEMBER>
class EventMap : private QMap<T, QMap<QString, SceneDocument > >
{
public:
    EventMap() ;
    void add(const SceneDocument& doc, const QString& collectionuid) ;
    SceneDocument remove(const QString& eventid) ;
    QList<SceneDocument> data(T key);
private:
    QString m_fieldname;
};

template <class T, char* MEMBER>
EventMap<T,MEMBER>::EventMap () : m_fieldname ( QString::fromAscii(MEMBER) ) { }

template <class T, char* MEMBER>
void EventMap<T,MEMBER>::add ( const SceneDocument& doc, const QString& collectionuid ) {
    T key = doc.getData().value ( m_fieldname ).value<T>();
    QMap<QString, SceneDocument > datas = QMap<T, QMap<QString, SceneDocument > >::take ( key );
    SceneDocument docm(doc);
	docm.setSceneid(collectionuid);
    datas.insert ( docm.id(), docm );
    QMap<T, QMap<QString, SceneDocument > >::insert ( key, datas );
}

template <class T, char* MEMBER>
SceneDocument EventMap<T,MEMBER>::remove ( const QString& eventid) {
    QMutableMapIterator<T, QMap<QString, SceneDocument > > outerMapIt(*this);
    while (outerMapIt.hasNext()) {
        outerMapIt.next();
        if (outerMapIt.value().contains(eventid)) {
	  const SceneDocument doc = outerMapIt.value().value(eventid);
            outerMapIt.value().remove(eventid);
            if (!outerMapIt.value().size()) {
                outerMapIt.remove();
            }
            return doc;
        }
    }
    return QVariantMap();
}

template <class T, char* MEMBER>
QList<SceneDocument> EventMap<T,MEMBER>::data(T key) {
    return QMap<T, QMap<QString, SceneDocument > >::value ( key ).values();
}
