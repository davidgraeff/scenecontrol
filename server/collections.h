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
#undef PLUGIN_ID
#define PLUGIN_ID "collection"
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include <shared/abstractserver.h>
#include "boolstuff/BoolExpr.h"

class ServiceController;
class CollectionInstance : public QObject {
	Q_OBJECT
public:
	CollectionInstance();
	virtual ~CollectionInstance();
	void startExecution() ;
    void stop();

	QSet<QString> eventids;
	boolstuff::BoolExpr<std::string>* conditionids;
	QMap< int, QString > actionids;
	bool enabled;
private:
	QTimer m_executionTimer;
	int m_currenttime;
private Q_SLOTS:
	void executiontimeout();
Q_SIGNALS:
	void executeService(const QString& uid);
};

class Collections : public QObject, public AbstractPlugin, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
public:
	Collections();
	virtual ~Collections();
	void setServiceController(ServiceController* sc) { m_servicecontroller = sc; }
    virtual void clear();
    virtual void initialize();
    virtual bool condition(const QVariantMap& data);
    virtual void event_changed(const QVariantMap& data);
    virtual void execute(const QVariantMap& data);
    virtual QList<QVariantMap> properties(const QString& sessionid);
private:
	void addCollection(const QVariantMap& data);
	ServiceController* m_servicecontroller;
	QMap< QString, CollectionInstance* > m_collections;
	void convertVariantToStringSet(const QVariantList& source, QSet<QString>& destination);
	void convertVariantToIntStringMap(const QVariantMap& source, QMap<int, QString>& destination);
public Q_SLOTS:
    void dataSync(const QVariantMap& data, const QString& sessionid = QString());
    void dataReady();
    void eventTriggered(const QString& uid);
Q_SIGNALS:
	void instanceExecute(const QString& uid);
};
