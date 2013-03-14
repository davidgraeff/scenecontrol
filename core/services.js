var api = require('./com/api.js').api;
var storage = require('./storage.js');

var servicelist = {};
var services = function() {
	this.servicelist = servicelist;
};
require('util').inherits(services, require('events').EventEmitter);

// Runtime variables
exports.services = new services();
exports.properties = {};
exports.variables = {};

exports.getService = function(uidDoc) {
	var id = uidDoc.componentid_ + "." + uidDoc.instanceid_;
	return servicelist[id];
}

exports.documentIsForService = function(doc, service) {
	var id = doc.componentid_ + "." +doc.instanceid_;
	return id == service.id;
}

exports.removeService = function(id) {
	if (!servicelist[id])
		return;
	exports.services.emit("removed", servicelist[id]);
	delete servicelist[id];
	console.log('Remove Service '+ id);
}

exports.addService = function(obj) {
	var id = obj.info.componentid_ + "." +obj.info.instanceid_;
	if (servicelist[id]) {
		exports.removeService(id);
	}
	
	var service = new exports.service(obj, id);
	servicelist[id] = service;
	
	console.log('New Service '+service.info.componentid_);
	exports.services.emit("added", service);
}

exports.service = function(com, id) {
	var that = this;
	that.id = id;
	that.com = com;
	that.info = com.info;
	
	// init service
	that.com.send(api.serviceAPI.init());
	// send all configurations that are for this service instance
	storage.db.collection('configuration').find({componentid_:that.info.componentid_,instanceid_:that.info.instanceid_}).toArray(function(err, items) {
		if (err) {
			return;
		}
		if (items) {
			items.forEach(function(configurationInstanceDoc) {
				that.com.send({method_: "instanceConfiguration", data: configurationInstanceDoc});
			});
		}
	});
	
	com.on("data", function(doc) {
		if (api.isAck(doc)) {
			that.emit("ack", doc.responseid_, doc.response_);
		} else if (api.isForStorage(doc)) {
			that.emit("modify_doc", doc, that.id);
		} else if (api.isServiceCall(doc)) {
			var dataDoc = api.getDataFromClientCall(doc, that.id);
			var remoteservice = exports.getService(dataDoc);
			if (!remoteservice) {
				console.warn("Service not found!", dataDoc);
				return;
			}

			if (remoteservice == that) {
				console.warn("Service addressed itself. Not delivering message!",that.info.componentid_);
				return;
			}
			remoteservice.com.send(dataDoc);
		} else if (api.isPropertyChange(doc)) {
			console.log('Property changed:', doc.id_);
			// we store properties -> can be used for variable value substitution in scene items
			exports.properties[doc.id_] = doc;
		} else if (api.isModelPropertyChange(doc)) {
// 			console.log('Property changed:', doc.id_);
		} else if (api.isTriggeredEvent(doc)) {
			that.emit("event_triggered", doc);
		} else 
			console.log('Unknown type:', doc);
	});
};
require('util').inherits(exports.service, require('events').EventEmitter);