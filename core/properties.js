var api = require('./com/api.js').api;

exports.variables = {};
exports.properties = {};
exports.propertyModels = {};

var propertyListener = {};
var propertyEvents = function() {};
require('util').inherits(propertyEvents, require('events').EventEmitter);
exports.propertyEvents = new propertyEvents();

exports.change = function(doc, from) {
	var data;
	var id = from.componentid_+""+from.instanceid_;
	
	if (!exports.properties[id])
		exports.properties[id] = {normal:{},model:{}};
	
	if (doc.method_) { // model property
		if (!exports.properties[id].model[doc.id_])
			exports.properties[id].model[doc.id_] = {};
		
		if (doc.method_=="change") {
			var info = exports.properties[id].model[doc.id_].info;
			if (!info)
				return;
			
			exports.properties[id].model[doc.id_].data[doc[info.key_]] = doc;
		}
		else if (doc.method_=="remove") {
			var info = exports.properties[id].model[doc.id_].info;
			if (!info)
				return;
			delete exports.properties[id].model[doc.id_].data[doc[info.key_]];
		}
		else if (doc.method_=="reset") {
			exports.properties[id].model[doc.id_] = {info:doc,data:{}}
		} else {
			console.warn("Properties: Model method not known!", doc.method_);
		}
	} else // normal property
		exports.properties[id].normal[doc.id_] = doc;
	
	for (var i in propertyListener) {
		var listener = propertyListener[i];
		console.log('Property propagate:', listener.info);
		if (listener.info != from)
			listener.send(doc);
	}
	
	exports.propertyEvents.emit("changed", doc);
}

exports.addPropertyListener = function(obj, id) {
	var d = propertyListener[id];
	if (d)
		return;
	propertyListener[id] = obj;
	obj.on("data", onPropertyListenerData);
	
	console.log('Add Property listener:', id);
}

exports.applyPropertiesAndVariables = function(doc, additionalVariables) {
	//TODO
// 	for (var i in exports.properties) {
// 		var serviceProperties = exports.properties[i];
// 		
// 		if (serviceProperties.normal) {
// 			for (var j in serviceProperties.normal) {
// 				var prop = serviceProperties.normal[j];
// 
// 			}
// 		}
// 	}
}

exports.setVariable = function(id, value) {
	exports.variables[id] = value;
}

exports.getVariable = function(id) {
	return exports.variables[id];
}


exports.removePropertyListener = function(id) {
	var d = propertyListener[id];
	if (!d)
		return;
	
	console.log('Remove Property listener:', id);
	d.removeListener("data", onPropertyListenerData);
	delete propertyListener[id];
}

function onPropertyListenerData(data, from) {
	if (api.consumerAPI.isRequestAllProperties(data)) {
		exports.requestAllProperties(function(items) {
			items.forEach(function(item) {
				from.send(item);
			});
		});
		return;
	}
// 	exports.change(data, from.info);
}

exports.requestAllProperties = function(callback) {
	var propertieslist = [];
	for (var i in exports.properties) {
		var serviceProperties = exports.properties[i];
		
		if (serviceProperties.normal) {
			for (var j in serviceProperties.normal) {
				propertieslist.push(serviceProperties.normal[j]);
			}
		}
		
		
		if (serviceProperties.model) {
			for (var i in serviceProperties.model) {
				var model = serviceProperties.model[i];
				propertieslist.push(model.info);
				for (var j in model.data) {
					propertieslist.push(model.data[j]);
				}
			}
		}
	}

	callback(propertieslist);
}
