var api = require('./com/api.js').api;

var servicelist = {};
var services = function() {
	this.servicelist = servicelist;
};

require('util').inherits(services, require('events').EventEmitter);
exports.services = new services();
exports.properties = {};

exports.getService = function(componentid, instanceid) {
	var id = componentid + "." +instanceid;
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
	
	com.on("data", function(doc) {
		if (api.isAck(doc)) {
			that.emit("ack", doc.responseid_);
		} else if (api.isForStorage(doc)) {
			that.emit("modify_doc", doc, that.id);
		} else if (api.isServiceCall(doc)) {
			that.emit("service_call", api.getDataFromClientCall(doc, that.id));
		} else if (api.isPropertyChange(doc)) {
			console.log('Property changed:', doc.id_);
			// we store properties -> can be used for variable value substitution in scene items
			exports.properties[doc.id_] = doc;
		} else if (api.isModelPropertyChange(doc)) {
// 			console.log('Property changed:', doc.id_);
		} else 
			console.log('Unknown type:', doc);
	});
};
require('util').inherits(exports.service, require('events').EventEmitter);