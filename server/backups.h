/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

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

	Purpose: Create, load, remove backups of all service files
*/

#pragma once
#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QDir>

class Backups: public QObject
{
    Q_OBJECT
public:
    Backups ();
    virtual ~Backups();
	void create(const QString& name);
	void rename(const QString& id, const QString& name);
	void restore ( const QString& id );
	void remove ( const QString& id );
	QStringList list();
private:
	QDir m_savedir;
};
