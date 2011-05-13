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
#include "shared/pluginservicehelper.h"
#include <shared/abstractserver.h>

class ServiceController;
class CollectionInstance : public QObject {
    Q_OBJECT
public:
    CollectionInstance( ServiceController* sc);
    virtual ~CollectionInstance();
	void setData(const QVariantMap& data);
	void registerEvent(const QVariantMap& event);
	void unregisterEvent(const QVariantMap& event);
	void reregisterEvents(const QString& eventuid = QString());
    void startExecution() ;
    void stop();
	void clone();

    QSet<QString> eventids;
    QSet<QString> conditionids;
	QSet<QString> actionids;
    QMap< int, QString > executionids;
	QMap<QString, QVariantMap> m_eventdataCache;
    bool enabled;
	QString collectionuid;
private:
    QTimer m_executionTimer;
    int m_currenttime;
	ServiceController* m_servicecontroller;
private Q_SLOTS:
    void executiontimeout();
Q_SIGNALS:
    void executeService ( const QString& uid, const QString& sessionid );
};

class Collections : public QObject, public AbstractPlugin, public AbstractPlugin_services {
    Q_OBJECT
    PLUGIN_MACRO
public:
    Collections();
    virtual ~Collections();
    void setServiceController ( ServiceController* sc ) {
        m_servicecontroller = sc;
    }
    virtual void clear();
    virtual void initialize();
    virtual bool condition ( const QVariantMap& data, const QString& sessionid );
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid );
	virtual void unregister_event ( const QVariantMap& data, const QString& collectionuid );
    virtual void execute ( const QVariantMap& data, const QString& sessionid );
    virtual QList<QVariantMap> properties ( const QString& sessionid );
    static void convertVariantToStringSet ( const QVariantList& source, QSet<QString>& destination );
    static void convertVariantToIntStringMap ( const QVariantMap& source, QMap<int, QString>& destination );
private:
    void addCollection ( const QVariantMap& data );
    ServiceController* m_servicecontroller;
    QMap< QString, CollectionInstance* > m_collections;
public Q_SLOTS:
    void dataSync ( const QVariantMap& data, const QString& sessionid = QString() );
    void dataReady();
    void eventTriggered(const QString& event_id, const QString& destination_collectionuid);
Q_SIGNALS:
    void instanceExecute ( const QString& uid, const QString& sessionid );
};
