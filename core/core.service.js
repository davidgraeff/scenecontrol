var api = require('./com/api.js').api;
var clientcom = require('./com/clientcom.js').clientcom;
var services = require('./services.js');
var scenes = require("./scenes.js");
var properties = require('./properties.js');

var coreservice = function() {
	var that = this;
	this.com = new clientcom(this);
	this.com.identified({componentid_:"core",instanceid_:"main",provides:["service"]});
	
	var registeredEvents_startscene = {};
	var registeredEvents_stopscene = {};
	
	this.com.send = function(data) {
		var m = data.method_;
// 		console.log("core service:", m);
		if (m == "startscene") {
			var scene = scenes.getScene(data.sceneid_);
			if (scene) {
				scene.startScene();
			}
			that.com.receive(api.generateAck(data,(scene!=undefined)));
			
		} else if (m == "stopscene") {
			var scene = scenes.getScene(data.sceneid_);
			if (scene) {
				scene.stopScene();
			}
			that.com.receive(api.generateAck(data,(scene!=undefined)));
			
		} else if (m == "onstartscene") {
			registeredEvents_startscene[data.id_+data.sceneid_] = data;
			that.com.receive(api.generateAck(data,true));
			
		} else if (m == "onstopscene") {
			registeredEvents_stopscene[data.id_+data.sceneid_] = data;
			that.com.receive(api.generateAck(data,true));
			
		} else if (m == "compareVariable" && data.var_id && data.value && data.cmpOp) {
			var cmpOp = data.cmpOp;
			var storedValue = properties.getVariable(data.var_id);
			if (cmpOp=="==")
				that.com.receive(api.generateAck(data,data.value==storedValue));
			else if (cmpOp=="<")
				that.com.receive(api.generateAck(data,data.value<storedValue));
			else if (cmpOp==">")
				that.com.receive(api.generateAck(data,data.value>storedValue));
			else
				that.com.receive(api.generateAck(data,false));
			
		} else if (m == "setVariable" && data.var_id && data.value) {
			that.com.receive(api.generateAck(data,properties.setVariable(data.var_id, data.value)));
			
		} else if (m == "getVariable" && data.var_id) {
			that.com.receive(api.generateAck(data,properties.getVariable(data.var_id)));
		}
	}
};


exports.init = function(callback) {
	exports.service = new coreservice();
	callback();
}

exports.finish = function(callback) {
	exports.service.com.free();
	exports.service = null;
	callback();
}

/*
QByteArray m = data.method();
bool response = true;
if (m=="delay") {
	// Delay execution of the current thread for x milliseconds
	msleep(data.getData().value(QLatin1String("waitms"),0).toInt());
	
} else if (m=="javascript") {
	//QScriptEngine e;
	
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
}*/