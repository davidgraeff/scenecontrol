#pragma once
/*
 *  RoomControlServer. Home automation for controlling sockets, leds and music.
 *  Copyright (C) 2012  David Gräff
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Purpose: SceneNode
*/
#include <shared/jsondocuments/scenedocument.h>

/**
 * A generic scene node. Refer to a scene document by type+id (=unique id)
 */
class SceneNode: public QObject {
	Q_OBJECT
public:
	/**
	 * Create an empty node: Use this method for creating a root node
	 */
	static SceneNode* createEmptyNode();
	/**
	 * We do not store the scene document but only the unqiue id and all other information that
	 * is provided by the scene
	 */
	static SceneNode* createNode(SceneDocument::TypeEnum type, const QString& id, const QVariantList& nextNodes, const QVariantList& alternativeNextNodes) ;
	
	SceneDocument::TypeEnum getType() const;
	QString getID() const;
	
	void setNextNodes(const QList< SceneDocument >& nextNodes);
	/**
	 * Run this node and return next nodes
	 */
	QList<SceneDocument> run();
private Q_SLOTS:
	void pluginResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid) ;
	void timeout() {}
protected:
	SceneDocument::TypeEnum mType;
	QString mID;
	QList<SceneDocument> mNextNodes;
	QList<SceneDocument> mNextAlternativeNodes;
	QAtomicInt mPluginResponseAvailable;
	bool mResponse;
	explicit SceneNode(SceneDocument::TypeEnum type, const QString& id, const QList< SceneDocument >& nextNodes, const QList< SceneDocument >& alternativeNextNodes) ;
	public slots:
};
