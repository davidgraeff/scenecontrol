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
#include "abstractplugin.h"
#include <QDateTime>

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:

    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties();
	
	struct VariableEvent {
		QByteArray name;
		QString eventid;
		QString sceneid;
		QVariant compareValue;
	};
public Q_SLOTS:
//     void eventDateTime ( const QString& id_, const QString& sceneid_, const QString& date, const QString& time);

private:

	void removeEvent ( const QString& eventid);
	void addToEvents(const VariableEvent& ts);
	QMap<QByteArray, VariableEvent> mEvents;
	
	void msleep(unsigned long msecs);
	// For Variables (persistant for the server process lifetime)
	QMap<QByteArray, QByteArray> textVariables;
	QMap<QByteArray, int> numberVariables;
	QList<VariableEvent> mVariableEvents;
private Q_SLOTS:
};
