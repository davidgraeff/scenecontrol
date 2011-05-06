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

void plugin::initialize(){
    calculate_next_events();
}

void plugin::clear() {
    m_timer.stop();
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
}

void plugin::execute ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED ( data );
	Q_UNUSED ( sessionid );
}

bool plugin::condition ( const QVariantMap& data, const QString& sessionid )  {
	Q_UNUSED ( sessionid );
    if ( ServiceID::isId(data, "datespan" ) ) {
        QDate m_lower = QDate::fromString ( DATA ( "lower" ),Qt::ISODate );
        QDate m_upper = QDate::fromString ( DATA ( "upper" ),Qt::ISODate );
        if ( QDate::currentDate() < m_lower ) return false;
        if ( QDate::currentDate() > m_upper ) return false;
        return true;
    } else
    if ( ServiceID::isId(data, "timespan" ) ) {
        QTime m_lower = QTime::fromString ( DATA ( "lower" ),Qt::ISODate );
        QTime m_upper = QTime::fromString ( DATA ( "upper" ),Qt::ISODate );
        if ( QTime::currentTime() < m_lower ) return false;
        if ( QTime::currentTime() > m_upper ) return false;
        return true;
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid ) {
	Q_UNUSED ( collectionuid );
    const QString uid = ServiceType::uniqueID(data);
    // remove from next events
    m_timeout_events.remove ( SC_Uid(uid, collectionuid) );
    // recalculate next event
    m_remaining_events[uid] = ServiceType::newDataWithCollectionUid(data, collectionuid);
    calculate_next_events();
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
	Q_UNUSED(collectionuid);
	const QString uid = ServiceType::uniqueID(data);
    // remove from next events
    m_timeout_events.remove ( SC_Uid(uid, collectionuid) );
    // recalculate next event
    calculate_next_events();
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    return l;
}

void plugin::timeout() {
    // events triggered, propagate to server
    foreach ( QVariantMap data, m_timeout_events ) {
        m_server->event_triggered ( ServiceType::uniqueID(data), ServiceType::getCollectionUid(data) );
    }
    m_timeout_events.clear();
    // calculate next events
    calculate_next_events();
}

void plugin::calculate_next_events() {
    QMap<int, DataBySC_Uid > min_next_time;
    QSet<QString> remove;

    foreach ( QVariantMap data, m_remaining_events ) {
        if ( ServiceID::isId(data, "timedate" ) ) {
			const QTime time = QTime::fromString ( DATA ( "time" ),Qt::ISODate );
			const QDate date = QDate::fromString ( DATA ( "date" ),Qt::ISODate );
            const int sec = QDateTime::currentDateTime().secsTo ( QDateTime(date, time) );
            if ( sec > 86400 ) {
                qDebug() << "One-time alarm: Armed (next check only)";
                min_next_time[86400];
            } else if ( sec > 10 ) {
                qDebug() << "One-time alarm: Armed" << sec;
                min_next_time[sec].insert ( SC_Uid(ServiceType::uniqueID(data), ServiceType::getCollectionUid(data)), data );
                remove.insert ( ServiceType::uniqueID(data) );
            } else if ( sec > -10 && sec < 10 ) {
                m_server->event_triggered ( ServiceType::uniqueID(data), ServiceType::getCollectionUid(data) );
                remove.insert ( ServiceType::uniqueID(data) );
            }
        } else if ( ServiceID::isId(data, "timeperiodic" ) ) {
            QTime m_time = QDateTime::fromString ( DATA ( "time" ),Qt::ISODate ).time();
            QDateTime datetime = QDateTime::currentDateTime();
            datetime.setTime ( m_time );
            bool days[7];
            int dayint = INTDATA ( "days" );
            for ( int i=0;i<7;++i ) {
                days[i] = dayint & ( 1<<i );
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

            if ( offsetdays < 7 ) {
                const int sec = QDateTime::currentDateTime().secsTo ( datetime.addDays ( offsetdays ) ) + 1;
				min_next_time[sec].insert ( SC_Uid(ServiceType::uniqueID(data), ServiceType::getCollectionUid(data)), data );
                qDebug() << "Periodic alarm: " << datetime.addDays ( offsetdays ).toString ( Qt::DefaultLocaleShortDate );
            }
        }
    }

    // remove remaining events that are in the next event list
    foreach (QString uid, remove)
		m_remaining_events.remove ( uid );

    if ( min_next_time.size() > 0 ) {
        // add entry to next events
		QMap<int, DataBySC_Uid >::const_iterator i = min_next_time.lowerBound(0);
		m_timeout_events = i.value();
        // start timer
        m_timer.start ( i.key() * 1000 );
    }
}
