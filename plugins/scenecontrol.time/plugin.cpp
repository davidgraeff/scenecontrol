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
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid) {
    m_timer.setSingleShot ( true );
    connect ( &m_timer, SIGNAL ( timeout() ), SLOT ( timeout() ) );
}

plugin::~plugin() {

}

void plugin::initialize() {
    //calculate_next_events();
}

void plugin::clear() {
    m_timer.stop();
	m_remaining_events.clear();
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
    // recalculate next event
    EventTimeStructure s;
    s.sceneid = sceneid_;
    s.date = QDate::fromString(date, Qt::ISODate);
    s.time = QTime::fromString(time, QLatin1String("h:m"));
    m_remaining_events[id_] = s;
    calculate_next_events();

    // property update
    SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
    sc.setData("uid", id_);
    changeProperty(sc.getData());
}

void plugin::eventPeriodic ( const QString& id_, const QString& sceneid_, const QString& time, const QVariantList& days) {
	QVariantList days_ = days;
	if (days.size()<7)
		return;
	
	QBitArray converted(7);
	for (int i=0;i < 7;++i) {
		converted.setBit(i, days.at(i).toBool());
	}
	
// qDebug() << __FUNCTION__ << time << converted.testBit(0) << converted.testBit(1) << converted.testBit(2) << converted.testBit(3);
    // recalculate next event
    EventTimeStructure s;
    s.sceneid = sceneid_;
    s.time = QTime::fromString(time, QLatin1String("h:m"));
    s.days = converted;
    m_remaining_events[id_] = s;
    calculate_next_events();

    // property update
    SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
    sc.setData("uid", id_);
    changeProperty(sc.getData());
}

void plugin::unregister_event ( const QString& eventid) {
    // remove from remaining events
    m_remaining_events.remove(eventid);

    // recalculate next event
    calculate_next_events();

    // property update
    SceneDocument s = SceneDocument::createModelRemoveItem("time.alarms");
    s.setData("uid", eventid);
    changeProperty(s.getData());
}

void plugin::requestProperties(int sessionid) {
        SceneDocument s = SceneDocument::createNotification("nextalarm");
		if (m_nextalarm.isValid())
			s.setData("seconds", QDateTime::currentDateTime().secsTo(m_nextalarm));
        changeProperty(s.getData(), sessionid);

    changeProperty(SceneDocument::createModelReset("time.alarms", "uid").getData(), sessionid);

    QMap<QString, EventTimeStructure>::iterator i = m_remaining_events.begin();
    for (;i != m_remaining_events.end(); ++i) {
        SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
        sc.setData("uid", i.key());
        changeProperty(sc.getData(), sessionid);
    }
}

void plugin::timeout() {
    // calculate next events
    calculate_next_events();
}

bool plugin::calculate_next_timer_timeout(const int seconds, int& nextTime, const QString& eventid, const EventTimeStructure& eventtime) {
	if ( seconds > 86400 ) {
		if (nextTime==-1) nextTime = 86400;
	} else if ( seconds > 10 ) {
		qDebug() << "One-time alarm: Armed" << seconds;
		if (nextTime==-1 || seconds<nextTime) nextTime = seconds;
	} else if ( seconds > -10 && seconds < 10 ) {
		qDebug() << "One-time alarm: Triggered" << eventtime.sceneid;
		eventTriggered ( eventid.toAscii(), eventtime.sceneid.toAscii() );
		return true;
	} else {
		qDebug() << "One-time alarm: Remove" << eventtime.sceneid << seconds;
		return true;
	}
	return false;
}

void plugin::calculate_next_events() {
	int nextTime = -1;
    QSet<QString> removeEventids;

    QMap<QString, EventTimeStructure>::iterator i = m_remaining_events.begin();
    for (;i != m_remaining_events.end(); ++i) {
      const QString& eventid = i.key();
      const EventTimeStructure& eventtime = i.value();
      
        if ( !eventtime.date.isNull() ) {
            const QDateTime datetime(eventtime.date, eventtime.time);
            const int sec = QDateTime::currentDateTime().secsTo ( datetime );
			if (calculate_next_timer_timeout(sec, nextTime, eventid, eventtime))
				removeEventids.insert ( eventid );
        } else if ( !eventtime.days.isEmpty() ) {
            QDateTime datetime = QDateTime::currentDateTime();
            datetime.setTime ( eventtime.time );
			
            int dow = QDate::currentDate().dayOfWeek() - 1;
            int offsetdays = 0;
			
            // If it is too late for the alarm today then
            // try with the next weekday
            if ( QTime::currentTime() > eventtime.time ) {
                dow = ( dow+1 ) % 7;
                ++offsetdays;
            }
			
            // Search for the next activated weekday
            for ( ; offsetdays < 8; ++offsetdays ) {
                if ( eventtime.days.testBit(dow) ) break;
                dow = ( dow+1 ) % 7;
            }

            if ( offsetdays < 8 ) {
                datetime = datetime.addDays ( offsetdays );
                const int sec = QDateTime::currentDateTime().secsTo ( datetime ) + 1;
				if (calculate_next_timer_timeout(sec, nextTime, eventid, eventtime))
					removeEventids.insert ( eventid );
            }
        }
    }

    // remove remaining events that are in the next event list
    foreach (QString uid, removeEventids) {
        m_remaining_events.remove ( uid );
        SceneDocument s = SceneDocument::createModelRemoveItem("time.alarms");
        s.setData("uid", uid);
        changeProperty(s.getData());
    }

    if ( nextTime != -1 ) {
        SceneDocument s = SceneDocument::createNotification("nextalarm");
		s.setData("seconds", nextTime);
		m_nextalarm = QDateTime::currentDateTime().addSecs(nextTime);
		m_timer.start ( nextTime * 1000 );
        changeProperty(s.getData());
    } else {
		m_nextalarm = QDateTime();
        SceneDocument s = SceneDocument::createNotification("nextalarm");
        changeProperty(s.getData());
    }
}
