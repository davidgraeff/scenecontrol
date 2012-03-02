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

	Purpose: CollectionController is a singleton object for
	(1) request execution of a collection
	(2) keep a list of running collections (to be able to them to e.g.)
	(3) Cache the last few executed collections
*/

#pragma once
#include <QVariantMap>

class RunningCollection;
/**
 * 	CollectionController is a singleton object for
 * (1) request execution of a collection
 * (2) keep a list of running collections (to be able to them to e.g.)
 * (3) Cache the last few executed collections
 */
class CollectionController: public QObject {
    Q_OBJECT
public:
	/// Singleton object
    static CollectionController* instance();
    virtual ~CollectionController();
private:
    CollectionController ();
	/**
	 * Keep a list of all running collections and also write them out if all properties are requested
	 * e.g. via requestExecution.
	 */
    QMap<QString, RunningCollection*> m_runningCollections;
	/**
	 * Cache a few collections. Although this application does not cache database documents
	 * in general to guarantee consistency, CollectionController is an exception because
	 * roundtrips to the database are expensive and not rational
	 * if the same collection is executed many times in sequal.
	 */ 
	QMap<QString, RunningCollection*> m_cachedCollections;
	/// Internally called after a running collection finished or aanother collection will be executed
    void updateListOfRunningCollections();
public Q_SLOTS:
	/**
	 * Request the execution of the collection identified by the collection id.
	 */
    void requestExecutionByCollectionId ( const QString& collectionid );
	/**
	 * Request execution of the document given by data. If no session id is known use -1.
	 * This slot is used to called by the network controller where a session id is always
	 * available.
	 */
    void requestExecution ( const QVariantMap& data, int sessionid );
	/**
	 * runningCollectionFinished is called by a RunningCollection after all actions have
	 * been executed. The collection will be taken out of m_runningCollections and cached
	 * as appropriate.
	 */
    void runningCollectionFinished (const QString& collectionid);
	/**
	 * Start a new running collection object.
	 * dataOfCollection is used to be called by the database and is an indirect reaction to
	 * a call to requestExecutionByCollectionId before.
	 */
    void dataOfCollection ( const QList<QVariantMap>& actions, const QList<QVariantMap>& conditions, const QString& collectionid);
};
