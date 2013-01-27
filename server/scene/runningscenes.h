#pragma once
#include <qmutex.h>
#include <QMap>
#include <QString>
#include <QSet>

class Scene;
class RunningScenes
{
public:
	static QMutex RunningScenesMutex;
	static RunningScenes* instance();
	virtual ~RunningScenes();
	void sceneThreadFinished (const QString& sceneid);
	void sceneThreadStarted (const QString& sceneid);
private:
	/**
	 * Keep a list of all running collections
	 */
	QSet<QString> mRunningScenes;
	void updateList();
};
