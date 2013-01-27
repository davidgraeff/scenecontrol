
#include "paths.h"
#include "scenecontroller.h"
#include "scene.h"
#include "scenenode.h"
#include "runningscenes.h"
#include "controlsocket/socket.h"
#include "shared/jsondocuments/scenedocument.h"
#include "libdatastorage/datastorage.h"
#include <QDebug>
#include <qthreadpool.h>
#define __FUNCTION__ __FUNCTION__

StorageNotifierScenes::StorageNotifierScenes(SceneController* sceneController) : mSceneController(sceneController)
{

}

void StorageNotifierScenes::documentChanged(const QString& /*filename*/, SceneDocument* /*oldDoc*/, SceneDocument* newDoc)
{
	if (newDoc->isType(SceneDocument::TypeScene)) {
		mSceneController->addOrChangeScene(newDoc);
	}
}

void StorageNotifierScenes::documentRemoved(const QString& /*filename*/, SceneDocument* document)
{
	if (document->isType(SceneDocument::TypeScene)) {
		mSceneController->removeScene(document);
	}
}

RunningScene::RunningScene(Scene* scene, SceneNode* node) : mScene(scene), mNode(node)
{

}

void RunningScene::run()
{
	const QList<SceneDocument> nextNodes = mNode->run();
	if (mStopNextNodeExecution)
		return;
	foreach(const SceneDocument& nn, nextNodes) {
		emit nextNode(mScene, nn.uid());
	}
	if (nextNodes.isEmpty())
		emit noNextNode(mScene);
}

void RunningScene::stopIfScene(Scene* sceneid)
{
	if (sceneid == mScene)
		mStopNextNodeExecution = 1;
}

static SceneController* ExecuteRequest_instance = 0;

SceneController* SceneController::instance() {
    if (!ExecuteRequest_instance) {
        ExecuteRequest_instance = new SceneController();
    }
    return ExecuteRequest_instance;
}

SceneController::SceneController () {
	mStorageNotifierScenes = new StorageNotifierScenes(this);
	DataStorage::instance()->registerNotifier(mStorageNotifierScenes);
}

SceneController::~SceneController() {
	delete mStorageNotifierScenes;
	qDeleteAll(mScenes);
	mScenes.clear();
}

void SceneController::load() {
	QList< SceneDocument* > documents = DataStorage::instance()->filteredDocuments(SceneDocument::TypeScene);
	foreach(SceneDocument* scene, documents)
		addOrChangeScene(scene);
}

void SceneController::startScene ( const QString& sceneid, const QString entryPointItemID ) {
	Scene* scene = mScenes.value(sceneid);
	if (!scene)
		return;
	
	if (startNode(scene, entryPointItemID)) {
		RunningScenes::instance()->sceneThreadStarted(sceneid);
	}
}

bool SceneController::startNode(Scene* scene, const QString& nodeUID)
{
	mRunningScenes.remove((RunningScene*)sender());
	SceneNode* node = 0;
	if (nodeUID.isEmpty())
		node = scene->getRootNode();
	else
		node = scene->getNode(nodeUID);
	
	if (!node)
		return false;
// 	qDebug() << __FUNCTION__ << scene->scenedoc()->toString("name") << node->getNextNodeUIDs();
	
	RunningScene* rScene = new RunningScene(scene, node);
	connect(rScene, SIGNAL(nextNode(Scene*,QString)), SLOT(startNode(Scene*,QString)));
	connect(rScene, SIGNAL(noNextNode(Scene*)), SLOT(sceneFinished(Scene*)));
	mRunningScenes.insert(rScene);
	QThreadPool::globalInstance()->start(rScene);
	return true;
}

void SceneController::stopScene(const QString& sceneid)
{
	Scene* run = mScenes.value(sceneid);
	if (run) {
		foreach(RunningScene* s, mRunningScenes) {
			s->stopIfScene(run);
		}
	}
}

void SceneController::removeScene(const SceneDocument* scene)
{
	if (scene->isType(SceneDocument::TypeScene)) {
		Scene* s = mScenes.take(scene->id());
		
		if (s) {
			delete s;
		}
	}
}

void SceneController::addOrChangeScene(const SceneDocument* scene) {
	Scene* s = mScenes.value(scene->id());
	if (s) {
		s->rebuild(scene);
	} else {
		mScenes.insert(scene->id(),new Scene(scene));
	}
}

void SceneController::sceneFinished(Scene* scene)
{
	mRunningScenes.remove((RunningScene*)sender());
	RunningScenes::instance()->sceneThreadFinished(scene->scenedoc()->id());
}
