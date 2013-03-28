var api = require('./com/api.js').api;
var clientcom = require('./com/clientcom.js').clientcom;
var services = require('./services.js');
var scenes = require("./sceneruntime.js");
var properties = require('./properties.js');
var variables = require('./variables.js');

var coreservice = function() {
	var that = this;
	// The clientcom class will register this core-service at the service list.
	this.com = new clientcom(this);
	this.com.identified({componentid_:"core",instanceid_:"main",provides:["service"]});
	
	var registeredEvents_startscene = {};
	var registeredEvents_stopscene = {};
	
	// List/Map of currently running timers (delay action)
	var delays = {};
	
	function abortDelay(sceneid) {
		if (!delays[sceneid])
			return;
		
		for (var k in delays[sceneid].actions) {
			clearTimeout(delays[sceneid].actions[k]);
		}
		
		delete delays[sceneid_];
	}
	
	function addDelay(sceneid, actionid, waitms) {
		if (!delays[data.sceneid_])
			delays[data.sceneid_] = {len:0,actions:{}}
		
		delays[data.sceneid_].len++;
		delays[data.sceneid_].actions[data.id_] = setTimeout(function() {
			delete delays[data.sceneid_].actions[data.id_];
			delays[data.sceneid_].len--;
			if (delays[data.sceneid_].len<=0)
				delete delays[data.sceneid_];
			that.com.receive(api.generateAck(data,true));
		}, waitms);
	}
	
	this.abort = function(sceneid) {
		abortDelay(data.sceneid_);
		that.com.receive(api.generateAck(data,true));
	}
	
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
			
		} else if (m == "delay") {
			addDelay(data.sceneid_, data.id_, data.waitms);
			
		} else if (m == "onstartscene") {
			registeredEvents_startscene[data.id_+data.sceneid_] = data;
			that.com.receive(api.generateAck(data,true));
			
		} else if (m == "onstopscene") {
			registeredEvents_stopscene[data.id_+data.sceneid_] = data;
			that.com.receive(api.generateAck(data,true));
			
		} else if (m == "compareVariable" && data.var_id && data.value && data.cmpOp) {
			var cmpOp = data.cmpOp;
			var storedValue = variables.getVariable(data.var_id);
			var res;
			switch(data.cmpOp) {
				case 0:
					res = data.value==storedValue;
					break;
				case 1:
					res = data.value<storedValue;
					break;
				case 2:
					res = data.value>storedValue;
					break;
				case 3:
					res = data.value<=storedValue;
					break;
				case 4:
					res = data.value>=storedValue;
					break;
				default:
					res = false;
			}
			that.com.receive(api.generateAck(data,res));
			
		} else if (m == "setVariable" && data.var_id && data.value) {
			// convert to number if necessary (type==0)
			if (data.type==0) {
				if (data.indexOf(",")!=-1||data.indexOf(".")!=-1)
					data.value = parseFloat(data.value);
				else
					data.value = parseInt(data.value);
			}
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
	try {
		exports.service.com.free();
	} catch(e) {
		console.warn(e);
	}
	exports.service = null;
	callback();
}
