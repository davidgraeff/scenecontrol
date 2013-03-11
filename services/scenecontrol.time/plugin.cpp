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
#include <QDateTime>
#include "plugin.h"
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<4) {
		qWarning()<<"Usage: plugin_id instance_id server_ip server_port";
		return 1;
	}
    
    if (plugin::createInstance(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return -1;
    return app.exec();
}

plugin::~plugin() {

}

void plugin::initialize() {
	m_timer.setSingleShot ( true );
	connect ( &m_timer, SIGNAL ( timeout() ), SLOT ( timeout() ) );
}

void plugin::clear() {
    m_timer.stop();
	mEvents.clear();
}

bool plugin::datespan ( const QString& current, const QString& lower, const QString& upper)  {
    QDate m_lower = QDate::fromString ( lower,Qt::ISODate );
    QDate m_upper = QDate::fromString ( upper,Qt::ISODate );
    QDate m_current = QDate::fromString ( current,Qt::ISODate );
    if ( m_current < m_lower ) return false;
    if ( m_current > m_upper ) return false;
    return true;
}

bool plugin::timespan ( const QString& current, const QString& lower, const QString& upper)  {
    QTime m_lower = QTime::fromString ( lower,Qt::ISODate );
    QTime m_upper = QTime::fromString ( upper,Qt::ISODate );
    QTime m_current = QTime::fromString ( current,Qt::ISODate );
    if ( m_current < m_lower ) return false;
    if ( m_current > m_upper ) return false;
    return true;
}

void plugin::eventDateTime ( const QString& id_, const QString& sceneid_, const QString& date, const QString& time) {
	if (sceneid_.isEmpty()) {
		removeEvent(id_);
		return;
	}
	
    // recalculate next event
    EventTimeStructure s;
	s.eventid = id_;
    s.sceneid = sceneid_;
    s.date = QDate::fromString(date, Qt::ISODate);
    s.time = QTime::fromString(time, QLatin1String("h:m"));
	QDateTime datetime(s.date, s.time);
	if (QDateTime::currentDateTime()>datetime)
		return;
	addToEvents(datetime, s);
}

void plugin::eventPeriodic ( const QString& id_, const QString& sceneid_, const QString& time, const QVariantList& days) {
	if (sceneid_.isEmpty()) {
		removeEvent(id_);
		return;
	}
	
	if (days.size()<7)
		return;
	
	QBitArray converted(7);
	for (int i=0;i < 7;++i) {
		converted.setBit(i, days.at(i).toBool());
	}
	
    // recalculate next event
    EventTimeStructure s;
	s.eventid = id_;
    s.sceneid = sceneid_;
    s.time = QTime::fromString(time, QLatin1String("h:m"));
    s.days = converted;
	calculate_next_periodic_timeout(s);
}

void plugin::removeEvent ( const QString& eventid) {
	SceneDocument s = SceneDocument::createModelRemoveItem("time.alarms");
	QMutableMapIterator<QDateTime, EventTimeStructure>  i(mEvents);
	while(i.hasNext()) {
		i.next();
		if (i.value().eventid == eventid) {
			// property update
			s.setData("uid", eventid);
			changeProperty(s.getData());
			i.remove();
		}
	}
	setupTimer();
}

void plugin::requestProperties(int sessionid) {
	SceneDocument s = SceneDocument::createNotification("nextalarm");
	if (mEvents.size())
		s.setData("seconds", QDateTime::currentDateTime().secsTo(mEvents.begin().key()));
	changeProperty(s.getData(), sessionid);

	changeProperty(SceneDocument::createModelReset("time.alarms", "uid").getData(), sessionid);

	QMapIterator<QDateTime, EventTimeStructure>  i(mEvents);
	while(i.hasNext()) {
		i.next();
        SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
        sc.setData("uid", i.value().eventid);
		sc.setData("seconds", QDateTime::currentDateTime().secsTo(i.key()));
        changeProperty(sc.getData(), sessionid);
    }
}

void plugin::timeout() {
	QMutableMapIterator<QDateTime, EventTimeStructure>  i(mEvents);
	while(i.hasNext()) {
		i.next();
		if (abs(QDateTime::currentDateTime().secsTo(i.key())) < 10) {
			plugin::EventTimeStructure ts = i.value();
			qDebug()<<"Alarm triggered"<<ts.sceneid;
			eventTriggered(ts.eventid, ts.sceneid);
			i.remove();
			if (ts.periodic)
				calculate_next_periodic_timeout(ts);
		}
	}
	setupTimer();
}

void plugin::setupTimer() {
	m_timer.stop();
	QMutableMapIterator<QDateTime, EventTimeStructure>  i(mEvents);
	while(i.hasNext()) {
		i.next();
		int sec = QDateTime::currentDateTime().secsTo(i.key());
		if (sec>0) {
			SceneDocument s = SceneDocument::createNotification("nextalarm");
			s.setData("seconds", sec);
			changeProperty(s.getData());
			qDebug()<<"Next alarm in"<<sec<<"s";
			m_timer.start ( sec * 1000 );
			break;
		}
		i.remove();
	};
}

void plugin::calculate_next_periodic_timeout(const plugin::EventTimeStructure& ts) {
	QDateTime t;
	QDateTime datetime = QDateTime::currentDateTime();
	datetime.setTime ( ts.time );
	
	int dow = QDate::currentDate().dayOfWeek() - 1;
	int offsetdays = 0;
	
	// If it is too late for the alarm today then
	// try with the next weekday
	if ( QTime::currentTime() > ts.time ) {
		dow = ( dow+1 ) % 7;
		++offsetdays;
	}
	
	// Search for the next activated weekday
	for ( ; offsetdays < 8; ++offsetdays ) {
		if ( ts.days.testBit(dow) ) break;
		dow = ( dow+1 ) % 7;
	}
	
	if ( offsetdays < 8 ) {
		addToEvents(datetime.addDays ( offsetdays ), ts);
	}	
}

void plugin::addToEvents(const QDateTime& nextTimeout, const plugin::EventTimeStructure& ts) {
	mEvents.insert(nextTimeout, ts);
	// property update
	SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
	sc.setData("uid", ts.eventid);
	sc.setData("seconds", QDateTime::currentDateTime().secsTo(nextTimeout));
	changeProperty(sc.getData());
	setupTimer();
}
