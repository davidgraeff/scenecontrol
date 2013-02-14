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

	Purpose: 
*/

#pragma once
#include <QMap>
#include <QObject>
#include <libdatastorage/datastorage.h>

class SceneNode;
class SceneDocument;

/**
 * Represent a scene and is build up by a scene document. A scene document contains scene items
 * which link to other scene items in a graph like structure. A whole scene or only parts of a scene
 * may be started by calling start. The execution is done in another thread to make the application
 * more robust to plugin delays and to allow wanted delay actions.
 */
class Scene: public AbstractStorageNotifierInterface {
    Q_OBJECT
public:
	Scene(const SceneDocument* scenedoc);
	virtual ~Scene();
	/**
	 * Start the entire scene by calling this method without an argument.
	 * If you provide a unique id the execution is limited to the scene item with
	 * the given unique id and its successive scene items.
	 */
	SceneNode* getRootNode();
	SceneNode* getNode(const QString entryPointUID);
	
	/**
	 * Return the related scene document
	 */
	const SceneDocument* scenedoc() const;
	/**
	 * Register events if enabled otherwise deregister events
	 */
	void setEnabled(bool en);
public Q_SLOTS:
	/**
	 * This is called by the scene document if that has been changed.
	 * This method (re)builds the scene node graph
	 */
	void rebuild(const SceneDocument* scenedoc = 0);
	/**
	 * If a plugin has been loaded, reregister all events belonging to that plugin
	 */
	void pluginInstanceLoaded(const QString& componentid, const QString& instanceid);
private:
	const SceneDocument* mScenedoc;
	/* A map from unique ids to scene nodes */
	QMap<QString, SceneNode*> mUID2SceneNode;
	/* The root SceneNode. Every node that does not have a predecessor have to be linked
	 * to this node, e.g. Events */
	SceneNode* mRootNode;
	/* Last time used */
	int m_lasttime;
	bool mEnabled;
	
	virtual void documentChanged(const QString& filename, SceneDocument* oldDoc, SceneDocument* newDoc);
	virtual void documentRemoved(const QString& filename, SceneDocument* document);
};
