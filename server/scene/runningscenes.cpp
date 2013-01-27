#include "runningscenes.h"
#include <controlsocket/socket.h>
#include <shared/jsondocuments/scenedocument.h>

RunningScenes* RunningScenes_instance = 0;
QMutex RunningScenes::RunningScenesMutex;

RunningScenes* RunningScenes::instance() {
	QMutexLocker locker(&RunningScenesMutex);
	if (!RunningScenes_instance) {
		RunningScenes_instance = new RunningScenes();
	}
	return RunningScenes_instance;
}

RunningScenes::~RunningScenes() {}

void RunningScenes::sceneThreadFinished(const QString& sceneid) {
	QMutexLocker locker(&RunningScenesMutex);
	mRunningScenes.remove(sceneid);
	updateList();
}
void RunningScenes::sceneThreadStarted(const QString& sceneid) {
	QMutexLocker locker(&RunningScenesMutex);
	mRunningScenes.insert(sceneid);
	updateList();
}

void RunningScenes::updateList() {
	SceneDocument doc = SceneDocument::createNotification("collection.running");
	QVariantList list;
	QList<QString> orig = mRunningScenes.values();
	for (int i=0;i<orig.size(); ++i) {
		list.append(orig[i]);
	}
	doc.setData("running",list);
	doc.setComponentID(QLatin1String("CollectionController"));
	//TODO: Thread issue -> use qt signal slot
	Socket::instance()->sendToClients(doc.getjson(), -1);
}
