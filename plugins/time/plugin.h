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
#include "_sharedsrc/abstractplugin.h"
#include <QDateTime>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin(const QString& instanceid);
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void unregister_event ( const QString& eventid);
public Q_SLOTS:
    void eventDateTime ( const QString& _id, const QString& collection_, const QString& date, const QString& time);
    void eventPeriodic ( const QString& _id, const QString& collection_, const QString& time, const QVariantList& days);
    bool datespan ( const QString& current, const QString& lower, const QString& upper);
    bool timespan ( const QString& current, const QString& lower, const QString& upper);
private:
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
    void calculate_next_events();
    struct EventTimeStructure {
        QString collectionuid;
        QDate date;
        QTime time;
        QBitArray days;
    };
    // eventid -> structure for a time event
    QMap<QString, EventTimeStructure> m_remaining_events;
    QMap<QString, EventTimeStructure> m_timeout_events;
    QTimer m_timer;
    QDateTime m_nextAlarm;
private Q_SLOTS:
    void timeout();
};
