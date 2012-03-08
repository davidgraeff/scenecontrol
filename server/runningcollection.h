/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2012  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Purpose: RunningCollection contains all action and condition documents to a given collection id.
	All actions are executed in the order of the action documents' _delay value respectivly but only
	if all condition documents are complied.
*/

#pragma once
#include <QVariantMap>
#include <QTimer>
#include <QPointer>

class PluginCommunication;

/**
 * RunningCollection contains all action and condition documents to a given collection id.
 * All actions are executed in the order of the action documents' _delay value respectivly but only
 * if all condition documents are complied.
 */
class RunningCollection: public QObject {
    Q_OBJECT
public:
    RunningCollection(const QList<QVariantMap>& actions, const QList<QVariantMap>& conditions, const QString& collectionid);
    void start();
	QString id() const;
private:
    struct dataWithPlugin {
        QPointer<PluginCommunication> plugin;
        QVariantMap data;
        dataWithPlugin(QPointer<PluginCommunication> p, QVariantMap m) : plugin(p), data(m) {}
    };
    QString m_collectionid;
    QTimer m_timer;
    int m_lasttime;
    QMultiMap<int, dataWithPlugin> m_timetable;
	QMultiMap<int, dataWithPlugin> m_runningtimetable;
    QList<dataWithPlugin> m_conditions;
    int m_conditionok;
private Q_SLOTS:
    void timeoutNextAction();
    void conditionResponse(bool timeout = false);
    void qtSlotResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid);
Q_SIGNALS:
    void runningCollectionFinished (const QString& collectionid);
};
