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
#include <QSet>
#include "shared/abstractplugin.h"
#include <QDateTime>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
    virtual void unregister_event ( const QString& eventid);
private:
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
    void calculate_next_events();
    typedef QPair<QString, QVariantMap> EventWithCollection;
    typedef QMap<QString, EventWithCollection> EventsWithCollectionMap;
    QMap<QString, EventWithCollection> m_remaining_events;
    EventsWithCollectionMap m_timeout_events;
    QTimer m_timer;
    QDateTime m_nextAlarm;
private Q_SLOTS:
    void timeout();
};
