var api = require('./com/api.js').api;
var clientcom = require('./com/clientcom.js').clientcom;
var services = require('./services.js');
var scenes = require("./sceneruntime.js");
var properties = require('./properties.js');
var variables = require('./variables.js');

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
			var storedValue = variables.getVariable(data.var_id);
			if (cmpOp=="==")
				that.com.receive(api.generateAck(data,data.value==storedValue));
			else if (cmpOp=="<")
				that.com.receive(api.generateAck(data,data.value<storedValue));
			else if (cmpOp==">")
				that.com.receive(api.generateAck(data,data.value>storedValue));
			else
				that.com.receive(api.generateAck(data,false));
			
		} else if (m == "setVariable" && data.var_id && data.value) {
			that.com.receive(api.generateAck(data,variables.setVariable(data.var_id, data.value)));
			
		} else if (m == "getVariable" && data.var_id) {
			that.com.receive(api.generateAck(data,variables.getVariable(data.var_id)));
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
