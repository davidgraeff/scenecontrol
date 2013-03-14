/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QDebug>
#include <QDateTime>
#include "plugin.h"
#include <QCoreApplication>
#include <qmutex.h>
#include <qwaitcondition.h>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<4) {
		qWarning()<<"Usage: instance_id server_ip server_port";
		return 1;
	}
    
    if (AbstractPlugin::createInstance<plugin>(PLUGIN_ID,argv[1],argv[2],argv[3])==0)
        return 10;
    return app.exec();
}

plugin::~plugin() {

}

void plugin::initialize() {

}

// Causes the current thread to sleep for msecs milliseconds.
void plugin::msleep(unsigned long msecs)
{
	QMutex mutex;
	mutex.lock();
	
	QWaitCondition waitCondition;
	waitCondition.wait(&mutex, msecs);
	
	mutex.unlock();
}

void plugin::clear() {

	mEvents.clear();
}

void plugin::removeEvent ( const QString& eventid) {
// 	SceneDocument s = SceneDocument::createModelRemoveItem("time.alarms");
// 	QMutableMapIterator<QDateTime, EventTimeStructure>  i(mEvents);
// 	while(i.hasNext()) {
// 		i.next();
// 		if (i.value().eventid == eventid) {
// 			// property update
// 			s.setData("uid", eventid);
// 			changeProperty(s.getData());
// 			i.remove();
// 		}
// 	}
}

void plugin::requestProperties() {
// 	SceneDocument s = SceneDocument::createNotification("nextalarm");
// 	if (mEvents.size())
// 		s.setData("seconds", QDateTime::currentDateTime().secsTo(mEvents.begin().key()));
// 	changeProperty(s.getData(), sessionid);
// 
// 	changeProperty(SceneDocument::createModelReset("time.alarms", "uid").getData(), sessionid);
// 
// 	QMapIterator<QDateTime, EventTimeStructure>  i(mEvents);
// 	while(i.hasNext()) {
// 		i.next();
//         SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
//         sc.setData("uid", i.value().eventid);
// 		sc.setData("seconds", QDateTime::currentDateTime().secsTo(i.key()));
//         changeProperty(sc.getData(), sessionid);
//     }
}

void plugin::addToEvents(const plugin::VariableEvent& ts) {
// 	mEvents.insert(nextTimeout, ts);
// 	// property update
// 	SceneDocument sc = SceneDocument::createModelChangeItem("time.alarms");
// 	sc.setData("uid", ts.eventid);
// 	sc.setData("seconds", QDateTime::currentDateTime().secsTo(nextTimeout));
// 	changeProperty(sc.getData());
}


bool operator==(plugin::VariableEvent const& lhs, plugin::VariableEvent const& rhs) {
	return lhs.name==rhs.name && lhs.eventid==rhs.eventid;
}

/*
void ServerProvidedFunctions::execute(const SceneDocument& data, const QByteArray& responseID, QObject* responseCallbackObject, int sessionid)
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
}*/