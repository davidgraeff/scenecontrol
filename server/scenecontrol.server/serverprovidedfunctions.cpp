#include "serverprovidedfunctions.h"
#include <scene/scenecontroller.h>
#include <QThread>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <QDebug>

QMap<QByteArray, QByteArray> ServerProvidedFunctions::textVariables;
QMap<QByteArray, int> ServerProvidedFunctions::numberVariables;
QList<ServerProvidedFunctions::VariableEvent> ServerProvidedFunctions::mVariableEvents;

bool operator==(ServerProvidedFunctions::VariableEvent const& lhs, ServerProvidedFunctions::VariableEvent const& rhs) {
	return lhs.name==rhs.name && lhs.eventid==rhs.eventid;
}


void ServerProvidedFunctions::execute(const SceneDocument& data, const QByteArray& responseID, QObject* responseCallbackObject, int /*sessionid*/)
{
	QByteArray m = data.method();
	bool response = true;
	if (m=="delay") {
		// Delay execution of the current thread for x milliseconds
		msleep(data.getData().value(QLatin1String("waitms"),0).toInt());
		
	} else if (m=="javascript") {
		//QScriptEngine e;
		
	} else if (m=="startscene") {
		if (!data.getData().contains(QLatin1String("sceneid")) ||
			!data.getData().contains(QLatin1String("sceneitemid"))) {
				qWarning()<<"ServerProvidedFunctions::execute::startscene missing arguments!";
				return;
		}
		SceneController::instance()->startScene(data.toString("sceneid"), data.toString("sceneitemid"));
	} else if (m=="stopscene") {
		if (!data.getData().contains(QLatin1String("sceneid"))) {
			qWarning()<<"ServerProvidedFunctions::execute::stopscene missing arguments!";
			return;
		}
		SceneController::instance()->stopScene(data.toString("sceneid"));
		
	} else if (m=="onSceneStarted") {
	} else if (m=="onSceneStopped") {
		
	} else if (m=="setTextVariable") {
		const QByteArray name = data.getData().value(QLatin1String("name")).toByteArray();
		if (executeFailure(m, "name", name.isEmpty())) return;
		const QByteArray value = data.getData().value(QLatin1String("value")).toByteArray();
		textVariables[name] = value;
		// check if an event is registered for this variable
		foreach(const VariableEvent& t, mVariableEvents ) {
			if (t.name==name && t.compareValue.toByteArray()==value) {
				SceneController::instance()->startScene(t.sceneid, t.eventid);
			}
		}
	} else if (m=="setNumberVariable") {
		const QByteArray name = data.getData().value(QLatin1String("name")).toByteArray();
		if (executeFailure(m, "name", name.isEmpty())) return;
		const int value = data.toInt("value");
		numberVariables[name] = value;
		// check if an event is registered for this variable
		foreach(const VariableEvent& t, mVariableEvents ) {
			if (t.name==name && t.compareValue.toInt()==value) {
				SceneController::instance()->startScene(t.sceneid, t.eventid);
			}
		}
		
	} else if (m=="isTextVariableEqual") {
		const QByteArray name = data.getData().value(QLatin1String("name")).toByteArray();
		if (executeFailure(m, "name", name.isEmpty())) return;
		const QByteArray value = data.getData().value(QLatin1String("value")).toByteArray();
		response = textVariables.value(name) == value;
	} else if (m=="isNumberVariableEqual") {
		const QByteArray name = data.getData().value(QLatin1String("name")).toByteArray();
		if (executeFailure(m, "name", name.isEmpty())) return;
		const int value = data.toInt("value");
		response = numberVariables.value(name) == value;
	} else if (m=="isNumberVariableWithin") {
		const QByteArray name = data.getData().value(QLatin1String("name")).toByteArray();
		if (executeFailure(m, "name", name.isEmpty())) return;
		const int lower = data.toInt("lower");
		const int upper = data.toInt("upper");
		response = numberVariables.value(name) <= upper && numberVariables.value(name) >= lower;
		
	} else if (m=="onTextVariable"||m=="onNumberVariable") {
		const QByteArray name = data.getData().value(QLatin1String("name")).toByteArray();
		if (executeFailure(m, "name", name.isEmpty())) return;
		const QVariant value = data.getData().value(QLatin1String("value"));
		const QString sceneid = data.sceneid();
		const QString eventid = data.id();
		VariableEvent t; t.name=name; t.eventid=eventid;t.sceneid=sceneid;t.compareValue=value;
		int i = mVariableEvents.indexOf(t);
		if (i!=-1) { // change or remove
			if (sceneid.isEmpty()) { // remove
				mVariableEvents.removeAt(i);
			} else { // change
				mVariableEvents[i] = t;
			}
		} else { // add
			mVariableEvents.append(t);
		}
	}
	if (responseCallbackObject) {
		QVariant responseV;
		responseV.setValue<bool>(response);
		ServerProvidedFunctions().emitSlotResponse(responseCallbackObject, responseV,responseID, data.componentID(), data.instanceID());
	}
}

bool ServerProvidedFunctions::executeFailure(const QByteArray& method, const QByteArray& argument, bool condition)
{
	if (condition) {
		qWarning()<<"ServerProvidedFunctions::execute::" << method << " missing argument:" << argument;
	}
	return condition;
}

void ServerProvidedFunctions::emitSlotResponse(QObject* responseCallbackObject, const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid)
{
	connect(this, SIGNAL(qtSlotResponse(QVariant,QByteArray,QString,QString)), responseCallbackObject,
			SLOT(pluginResponse(QVariant,QByteArray,QString,QString)));
	emit qtSlotResponse(response,responseid, pluginid, instanceid);
}

// Causes the current thread to sleep for msecs milliseconds.
void ServerProvidedFunctions::msleep(unsigned long msecs)
{
	QMutex mutex;
	mutex.lock();
	
	QWaitCondition waitCondition;
	waitCondition.wait(&mutex, msecs);
	
	mutex.unlock();
}