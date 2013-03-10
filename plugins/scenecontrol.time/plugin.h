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

#pragma once
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QBitArray>
#include <QSet>
#include "shared/plugins/abstractplugin.h"
#include <QDateTime>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:

    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
public Q_SLOTS:
    void eventDateTime ( const QString& id_, const QString& sceneid_, const QString& date, const QString& time);
    void eventPeriodic ( const QString& id_, const QString& sceneid_, const QString& time, const QVariantList& days);
    bool datespan ( const QString& current, const QString& lower, const QString& upper);
    bool timespan ( const QString& current, const QString& lower, const QString& upper);
	
private:
	void removeEvent ( const QString& eventid);
	struct EventTimeStructure {
		QString eventid;
        QString sceneid;
        QDate date;
        QTime time;
        QBitArray days;
		bool periodic;
    };
	void calculate_next_periodic_timeout(const EventTimeStructure& ts);
	void setupTimer();
	void addToEvents(const QDateTime& nextTimeout, const EventTimeStructure& ts);
	
    // eventid -> structure for a time event
	QMap<QDateTime, EventTimeStructure> mEvents;
    QTimer m_timer;
private Q_SLOTS:
    void timeout();
};
