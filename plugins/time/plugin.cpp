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
#include <QDateTime>

#include "plugin.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_timer.setSingleShot ( true );
    connect ( &m_timer, SIGNAL ( timeout() ), SLOT ( timeout() ) );
}

plugin::~plugin() {

}

void plugin::initialize() {
    calculate_next_events();
}

void plugin::clear() {
    m_timer.stop();
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED ( data );
    Q_UNUSED ( sessionid );
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "datespan" ) ) {
        QDate m_lower = QDate::fromString ( DATA ( "lower" ),Qt::ISODate );
        QDate m_upper = QDate::fromString ( DATA ( "upper" ),Qt::ISODate );
        if ( QDate::currentDate() < m_lower ) return false;
        if ( QDate::currentDate() > m_upper ) return false;
        return true;
    } else
        if ( ServiceID::isMethod(data, "timespan" ) ) {
            QTime m_lower = QTime::fromString ( DATA ( "lower" ),Qt::ISODate );
            QTime m_upper = QTime::fromString ( DATA ( "upper" ),Qt::ISODate );
            if ( QTime::currentTime() < m_lower ) return false;
            if ( QTime::currentTime() > m_upper ) return false;
            return true;
        }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    const QString eventid = ServiceID::id(data);
    // remove from next events
    m_timeout_events.remove ( eventid );

    // recalculate next event
    m_remaining_events[eventid] = EventWithCollection(collectionuid, data);
    calculate_next_events();

    // property update
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "time.alarms");
    sc.setData("uid", ServiceID::id(data));
    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(sessionid);
    // remove from next events
    m_timeout_events.remove ( eventid );

    // remove from remaining events
    m_remaining_events.remove(eventid);

    // recalculate next event
    calculate_next_events();

    // property update
    ServiceCreation s = ServiceCreation::createModelRemoveItem(PLUGIN_ID, "time.alarms");
    s.setData("uid", eventid);
    m_serverPropertyController->pluginPropertyChanged(s.getData());
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    if (!m_nextAlarm.isNull()) {
        ServiceCreation s = ServiceCreation::createNotification(PLUGIN_ID, "nextalarm");
        s.setData("date", m_nextAlarm.date().toString(QLatin1String("dd.MM.yyyy")));
        s.setData("time", m_nextAlarm.time().toString(QLatin1String("hh:mm")));
        l.append(s.getData());
    } else {
        ServiceCreation s = ServiceCreation::createNotification(PLUGIN_ID, "nextalarm");
        l.append(s.getData());
    }

    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "time.alarms", "uid").getData());

    foreach ( EventWithCollection edata, m_remaining_events ) {
        ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "time.alarms");
        sc.setData("uid", ServiceID::id(edata.second));
        l.append(sc.getData());
    }

    return l;
}

void plugin::timeout() {
    // events triggered, propagate to server
    foreach ( EventWithCollection edata, m_timeout_events ) {
        m_serverCollectionController->pluginEventTriggered ( ServiceID::id(edata.second), edata.first );
    }
    m_timeout_events.clear();

    // calculate next events
    calculate_next_events();
}

void plugin::calculate_next_events() {
    QMap<int, EventsWithCollectionMap > min_next_time;
    QSet<QString> remove;

    foreach ( EventWithCollection edata, m_remaining_events ) {
        const QVariantMap eventdata = edata.second;
	const QString eventid = ServiceID::id(eventdata);
	
        if ( ServiceID::isMethod(eventdata, "timedate" ) ) {
            const QTime time = QTime::fromString ( ServiceID::string(eventdata, "time" ), QLatin1String("h:m") );
            const QDate date = QDate::fromString ( ServiceID::string(eventdata, "date" ), QLatin1String("dd.MM.yyyy"));
            const QDateTime datetime(date, time);
            const int sec = QDateTime::currentDateTime().secsTo ( datetime );
            if ( sec > 86400 ) {
                min_next_time[86400];
            } else if ( sec > 10 ) {
                qDebug() << "One-time alarm: Armed" << sec;
                m_nextAlarm = datetime;
                min_next_time[sec].insert ( eventid, EventWithCollection(edata.first, eventdata) );
                remove.insert ( eventid );
            } else if ( sec > -10 && sec < 10 ) {
                m_serverCollectionController->pluginEventTriggered ( eventid, edata.first );
                remove.insert ( eventid );
            }
        } else if ( ServiceID::isMethod(eventdata, "timeperiodic" ) ) {
            QTime m_time = QDateTime::fromString ( ServiceID::string(eventdata, "time" ),QLatin1String("h:m") ).time();
            QDateTime datetime = QDateTime::currentDateTime();
            datetime.setTime ( m_time );
            bool days[7];
            {
                QList<QVariant> tempdays = eventdata.value ( QLatin1String("days") ).toList();
                for ( int i=0;i<7;++i ) {
                    days[i] = tempdays.contains(i);
                }
            }
            int dow = QDate::currentDate().dayOfWeek() - 1;
            int offsetdays = 0;

            // If it is too late for the alarm today then
            // try with the next weekday
            if ( QTime::currentTime() > m_time ) {
                dow = ( dow+1 ) % 7;
                ++offsetdays;
            }

            // Search for the next activated weekday
            for ( ; offsetdays < 8; ++offsetdays ) {
                if ( days[dow] ) break;
                dow = ( dow+1 ) % 7;
            }

            if ( offsetdays < 8 ) {
                datetime = datetime.addDays ( offsetdays );
                m_nextAlarm = datetime;
                const int sec = QDateTime::currentDateTime().secsTo ( datetime ) + 1;
                min_next_time[sec].insert ( eventid, EventWithCollection(edata.first, eventdata) );
            }
        }
    }

    // remove remaining events that are in the next event list
    foreach (QString uid, remove) {
        m_remaining_events.remove ( uid );
        ServiceCreation s = ServiceCreation::createModelRemoveItem(PLUGIN_ID, "time.alarms");
        s.setData("uid", uid);
        m_serverPropertyController->pluginPropertyChanged(s.getData());
    }

    if (!m_nextAlarm.isNull()) {
        ServiceCreation s = ServiceCreation::createNotification(PLUGIN_ID, "nextalarm");
        s.setData("date", m_nextAlarm.date().toString(QLatin1String("dd.MM.yyyy")));
        s.setData("time", m_nextAlarm.time().toString(QLatin1String("hh:mm")));
        m_serverPropertyController->pluginPropertyChanged(s.getData());
    } else {
        ServiceCreation s = ServiceCreation::createNotification(PLUGIN_ID, "nextalarm");
        m_serverPropertyController->pluginPropertyChanged(s.getData());
    }

    if ( min_next_time.size() > 0 ) {
        // add entry to next events
        QMap<int, EventsWithCollectionMap >::const_iterator i = min_next_time.lowerBound(0);
        m_timeout_events = i.value();
        // start timer
        m_timer.start ( i.key() * 1000 );
    }
}
