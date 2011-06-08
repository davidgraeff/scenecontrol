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
#undef PLUGIN_ID
#define PLUGIN_ID "backups"
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractserver.h>

class Backups: public QObject, public AbstractPlugin, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
public:
    Backups ();
    virtual ~Backups();
    virtual void clear();
    virtual void initialize();
    virtual bool condition(const QVariantMap& data, const QString& sessionid);
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid );
	virtual void unregister_event ( const QVariantMap& data, const QString& collectionuid );
    virtual void execute(const QVariantMap& data, const QString& sessionid);
    virtual QList<QVariantMap> properties(const QString& sessionid);
private:
	void create(const QString& name);
	void rename(const QString& id, const QString& name);
	void restore ( const QString& id );
	void remove ( const QString& id );
	struct Backup {
		QString id;
		QString name;
		QString date;
		int files;
	};
	QVariantMap changeNotify( const Backups::Backup& b, bool write = false);
	QMap<QString, Backup> m_backups;
};
