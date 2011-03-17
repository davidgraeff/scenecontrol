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

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() {
    m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
}

plugin::~plugin() {

}

void plugin::init(AbstractServer* server) {
	m_server=server;
	calculate_next_events();
}

void plugin::clear() {
	m_timer.stop();
}

void plugin::otherPropertyChanged(const QString& unqiue_property_id, const QVariantMap& value) {
Q_UNUSED(unqiue_property_id);Q_UNUSED(value);
}

void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
	PluginHelper::setSetting(name, value, init);
}

void plugin::execute(const QVariantMap& data) {
	Q_UNUSED(data);
}

bool plugin::condition(const QVariantMap& data)  {
	if (IS_ID("timespan")) {
		QTime m_lower = QTime::fromString(DATA("lower"),Qt::ISODate);
		QTime m_upper = QTime::fromString(DATA("upper"),Qt::ISODate);
		if (QTime::currentTime() < m_lower) return false;
		if (QTime::currentTime() > m_upper) return false;
		return true;
	}
	return false;
}

void plugin::event_changed(const QVariantMap& data) {
	const QString uid = DATA("uid");
	// remove from next events
	m_timeout_service_ids.remove(uid);
	// recalculate next event
	m_remaining_events[data[uid].toString()] = data;
	calculate_next_events();
}

QMap<QString, QVariantMap> plugin::properties() {
	QMap<QString, QVariantMap> l;
	return l;
}

void plugin::timeout() {
	// events triggered, propagate to server
	foreach(QString uid, m_timeout_service_ids) {
		m_server->event_triggered(uid);
	}
	m_timeout_service_ids.clear();
	// calculate next events
	calculate_next_events();
}

void plugin::calculate_next_events() {
	QMap<int, QSet<QString> > min_next_time;
	QSet<QString> remove;

	foreach(QVariantMap data, m_remaining_events) {
		if (IS_ID("timedate")) {
			const int sec = QDateTime::currentDateTime().secsTo (QDateTime::fromString(DATA("date"),Qt::ISODate));
			if (sec > 86400) {
				qDebug() << "One-time alarm: Armed (next check only)";
				min_next_time[86400*1000];
			} else if (sec > 10) {
				qDebug() << "One-time alarm: Armed" << sec;
				m_timer.start(sec*1000);
				min_next_time[sec*1000].insert(DATA("uid"));
				remove.insert(DATA("uid"));
			} else if (sec > -10 && sec < 10) {
				m_server->event_triggered(DATA("uid"));
				remove.insert(DATA("uid"));
			}
		} else if (IS_ID("timeperiodic")) {
			// timer triggered this timeout
			if (!aftersync && !m_aftertrigger)  {
				emit trigger();
				// get back in 2 seconds to generate the new periodic alarm
				m_aftertrigger = true;
				m_timer.start(2000);
				return;
			}
			m_aftertrigger = false;

			QTime m_time = QDateTime::fromString(DATA("time"),Qt::ISODate).time();
			QDateTime datetime = QDateTime::currentDateTime();
			datetime.setTime ( m_time );
			int dow = QDate::currentDate().dayOfWeek() - 1;
			int offsetdays = 0;

			// If it is too late for the alarm today then
			// try with the next weekday
			if ( QTime::currentTime() > m_time )
			{
				dow = (dow+1) % 7;
				++offsetdays;
			}

			// Search for the next activated weekday
			for (; offsetdays < 8; ++offsetdays )
			{
				if ( base->m_days[dow] ) break;
				dow = (dow+1) % 7;
			}

			if ( offsetdays >= 8 ) {
				return;
				qWarning()<<"Periodic alarm: Failure with weekday offset"<<offsetdays<<datetime.toString(Qt::DefaultLocaleShortDate);
			}

			const int sec = QDateTime::currentDateTime().secsTo (datetime.addDays ( offsetdays ));
			m_timer.start(sec*1000);
			qDebug() << "Periodic alarm: " << datetime.addDays ( offsetdays ).toString(Qt::DefaultLocaleShortDate);
		}
	}
	
	// remove remaining events that are in the next event list
	m_remaining_events.remove(remove);
	
	if (min_next_time.size() > 0) {
		// add entry to next events
		
		// start timer
		m_timer.start(ad);
	}
}
