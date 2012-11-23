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
    plugin(const QString& pluginid, const QString& instanceid);
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void unregister_event ( const QString& eventid);
public Q_SLOTS:
    void eventDateTime ( const QString& id_, const QString& sceneid_, const QString& date, const QString& time);
    void eventPeriodic ( const QString& id_, const QString& sceneid_, const QString& time, const QVariantList& days);
    bool datespan ( const QString& current, const QString& lower, const QString& upper);
    bool timespan ( const QString& current, const QString& lower, const QString& upper);
private:
    struct EventTimeStructure {
        QString sceneid;
        QDate date;
        QTime time;
        QBitArray days;
    };
	/**
	 * Return true if the event can be removed from the remaining events
	 */
	bool calculate_next_timer_timeout(const int seconds, int& nextTime, const QString& eventid, const EventTimeStructure& eventtime);
    void calculate_next_events();

    // eventid -> structure for a time event
    QMap<QString, EventTimeStructure> m_remaining_events;
    QTimer m_timer;
    QDateTime m_nextAlarm;
private Q_SLOTS:
    void timeout();
};
