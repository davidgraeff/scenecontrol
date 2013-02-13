#include "scene.h"
#include "scenenode.h"
#include "plugins/plugincontroller.h"
#include "shared/jsondocuments/scenedocument.h"
#include <libdatastorage/datastorage.h>


Scene::Scene(const SceneDocument* scenedoc): mScenedoc(scenedoc), mRootNode(0), m_lasttime(0)
{
	connect(PluginController::instance(), SIGNAL(pluginInstanceLoaded(QString,QString)), SLOT(pluginInstanceLoaded(QString,QString)));
    rebuild();
	setEnabled(scenedoc->toBool("enabled"));
}

Scene::~Scene()
{
	// If plugins are available, unregister our events
	if (PluginController::instance())
		setEnabled(false);
	qDeleteAll(mUID2SceneNode);
	mUID2SceneNode.clear();
	delete mRootNode;
}

void Scene::rebuild(const SceneDocument* scenedoc)
{
	// clean up
	setEnabled(false);
	qDeleteAll(mUID2SceneNode);
	mUID2SceneNode.clear();
	delete mRootNode;
	
	if (!mScenedoc && !scenedoc)
		return;
	
	// Only if the argument is set overwrite the existing scene document
	if (scenedoc)
		mScenedoc = scenedoc;
	
	// create new root node
	mRootNode = SceneNode::createEmptyNode();
	// we have to log all items and the referrenced items to determine all items without predecessor
	QSet<QString> nextNodeItems, allItems;
	QMap<QString, SceneDocument> itemsByUID;
	// Look at every scene item
	QVariantList l = mScenedoc->sceneItems();
	foreach(const QVariant& sceneItemData, l) {
		// Add scene item to the mUID2SceneNode list
		SceneDocument sceneItem(sceneItemData.toMap());
		QString uid = sceneItem.uid();
		mUID2SceneNode.insert(uid, SceneNode::createNode(sceneItem.type(), sceneItem.id(), sceneItem.nextNodes(), sceneItem.nextAlternativeNodes()));
		// Log to allItems and log all next nodes / alternative next nodes to nextNodeItems
		allItems.insert(uid);
		itemsByUID.insert(uid, sceneItem);
		QVariantList nextNodes = sceneItem.nextNodes() + sceneItem.nextAlternativeNodes();;
		foreach(const QVariant& nn, nextNodes)
			nextNodeItems.insert(SceneDocument(nn.toMap()).uid());
	}
	
	// Items without predecessor are added to the root node
	allItems.subtract(nextNodeItems);
	QList<SceneDocument> rootItems;
	foreach(const QString& uid, allItems) {
		rootItems.append(itemsByUID.values(uid));
	}
	mRootNode->setNextNodes(rootItems);
}

void Scene::setEnabled(bool en) {
	if (en) {
		DataStorage::instance()->registerNotifier(this);
	} else {
		DataStorage::instance()->unregisterNotifier(this);
	}
	mEnabled = en;
	foreach(SceneNode* node, mUID2SceneNode) {
		if (node->getType() != SceneDocument::TypeEvent)
			continue;
		
		SceneDocument doc = DataStorage::instance()->getDocumentCopy(SceneDocument::uid(node->getType(),node->getID()));
		if (!doc.isValid()) {
			qWarning() << "Scene::setEnabled(): Scene item is not valid!" << node->getID();
			continue;
		}
		
		if (en) {
			// register event
			doc.setSceneid(mScenedoc->id());
			PluginController::instance()->execute(doc);
		} else {
			// unregister event by setting the scene id to ""
			doc.setSceneid(QString());
			PluginController::instance()->execute(doc);
		}
	}
}

SceneNode* Scene::getNode(const QString entryPointUID)
{
	return mUID2SceneNode.value(entryPointUID);
}

SceneNode* Scene::getRootNode()
{
	return mRootNode;
}


const SceneDocument* Scene::scenedoc() const
{
	return mScenedoc;
}

void Scene::documentChanged(const QString& /*filename*/, SceneDocument* oldDoc, SceneDocument* newDoc)
{
	if (oldDoc && newDoc->isType(SceneDocument::TypeEvent) && mUID2SceneNode.contains(newDoc->uid())) {
		// unregister
		SceneDocument oE(oldDoc->getData());
		oE.setSceneid(QString());
		PluginController::instance()->execute(oE);
		
		// register
		if (mEnabled && newDoc)
			PluginController::instance()->execute(*newDoc);
	}
}

void Scene::documentRemoved(const QString& /*filename*/, SceneDocument* document)
{
	if (document->isType(SceneDocument::TypeEvent) && mUID2SceneNode.contains(document->uid())) {
		// unregister
		SceneDocument oE(document->getData());
		oE.setSceneid(QString());
		PluginController::instance()->execute(oE);
	}
}

void Scene::pluginInstanceLoaded(const QString& componentid, const QString& instanceid)
{
	// If scene is enabled look at all scnene items for events that belong to this loaded plugin instance
	if (!mEnabled) {
		return;
	}
	
	foreach(SceneNode* node, mUID2SceneNode) {
		if (node->getType() != SceneDocument::TypeEvent)
			continue;
		
		SceneDocument doc = DataStorage::instance()->getDocumentCopy(SceneDocument::uid(node->getType(),node->getID()));
		if (!doc.isValid()) {
			qWarning() << "Scene::pluginInstanceLoaded(): Scene item is not valid!" << node->getID();
			continue;
		}
		
		if (doc.componentID()!=componentid || doc.instanceID() != instanceid) {
			continue;
		}

		// register event
		doc.setSceneid(mScenedoc->id());
		PluginController::instance()->execute(doc);
	}
}
