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

*/

#pragma once
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractserver.h>

class ServiceController;
class ServiceStruct;
class CollectionInstance : public QObject {
    Q_OBJECT
public:
    CollectionInstance( ServiceStruct* service, ServiceController* sc);
    virtual ~CollectionInstance();
    QString name();
	void change(const QVariantMap& data, const QVariantMap& olddata);
	void clone();
	bool removeService ( const QString& uid );
	bool changeService ( ServiceStruct* service, const QVariantMap& data, const QVariantMap& olddata );
	bool containsService ( const QString& uid );

	ServiceStruct* serviceStruct();
	
    void start();
    void stop();
	void execute();
	
private:
	ServiceStruct* m_collection;
	ServiceController* m_servicecontroller;
    QMap<QString, ServiceStruct*> m_serviceids;
    QMap< int, ServiceStruct* > m_executionids;
    bool m_enabled;
    QTimer m_executionTimer;
    int m_currenttime;
	void updateServiceIDs();
private Q_SLOTS:
    void executiontimeout();
};
