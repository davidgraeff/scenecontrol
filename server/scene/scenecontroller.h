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
#include <QVector>
#include <qrunnable.h>
#include <QAtomicInt>
#include <libdatastorage/datastorage.h>

class Scene;
class SceneNode;
class SceneController;

class StorageNotifierScenes: public AbstractStorageNotifier {
public:
	StorageNotifierScenes(SceneController* sceneController);
private:
	SceneController* mSceneController;
	// Called by the DataStorage
	virtual void documentChanged(const QString& filename, SceneDocument* oldDoc, SceneDocument* newDoc);
	// Called by the DataStorage
	virtual void documentRemoved(const QString& filename, SceneDocument* document);
};


class RunningScene: public QObject, public QRunnable {
	Q_OBJECT
public:
	RunningScene(Scene* scene, SceneNode* node);
	virtual void run();
	void stopIfScene(Scene* sceneid);
private:
	Scene* mScene;
	SceneNode* mNode;
	QAtomicInt mStopNextNodeExecution;
Q_SIGNALS:
	void nextNode(Scene* scene, const QString& nextNode);
	void noNextNode(Scene* scene);
};


class Scene;
/**
 * 	SceneController is a singleton object for
 * (1) request starting/stopping of a scene
 * (2) keep a list of running scenes
 * (3) a list of all scene objects
 */
class SceneController: public QObject {
    Q_OBJECT
    friend class StorageNotifierScenes;
public:
	/// Singleton object
    static SceneController* instance();
    virtual ~SceneController();
	/**
	 * Get scenes from the database and build graphs of all scene structures in memory
	 */
	void load();
private:
    SceneController ();
	StorageNotifierScenes* mStorageNotifierScenes;
	QMap<QString, Scene*> mScenes;
	QSet<RunningScene*> mRunningScenes;

	void addOrChangeScene(const SceneDocument* scene );
	void removeScene( const SceneDocument* scene );
public Q_SLOTS:
	/**
	 * Request the execution of the scene identified by the scene id. An entry point may be set optionally
	 */
    void startScene ( const QString& sceneid, const QString entryPointItemID = QString() );
	void stopScene (const QString& sceneid);
	void startNode( Scene* scene, const QString& nodeUID);
	void sceneFinished (Scene* scene);
};
