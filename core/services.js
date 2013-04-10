var api = require('./com/api.js').api;
var storage = require('./storage.js');
var properties = require('./properties.js');

// start phase
var startphase = true;
setTimeout(function() { startphase = false; }, 2000);

var servicelist = {};
var services = function() {
	this.servicelist = servicelist;
};
require('util').inherits(services, require('events').EventEmitter);

// Runtime variables
exports.services = new services();

exports.getService = function(uidDoc) {
	return servicelist[exports.serviceid(uidDoc)];
}

exports.documentIsForService = function(doc, service) {
	return exports.serviceid(doc) == service.id;
}

exports.removeService = function(uidDoc) {
	var id = exports.serviceid(uidDoc);
	if (!servicelist[id])
		return;
	exports.services.emit("removed", servicelist[id]);
	servicelist[id] = null;
	delete servicelist[id];
	console.log('Remove Service '+ id);
}

exports.serviceid = function(uidDoc) {
	return uidDoc.componentid_ + "." +uidDoc.instanceid_;
}

exports.addService = function(obj, uidDoc) {
	var id = exports.serviceid(uidDoc);
	if (servicelist[id]) {
		exports.removeService(uidDoc);
	}
	
	var service = new exports.service(obj, id);
	servicelist[id] = service;
	
	console.log('New Service: '+service.info.componentid_);
	exports.services.emit("added", service);
}

exports.servicecall = function(doc, clientcom) {
	var dataDoc = api.serviceAPI.getDataFromClientCall(doc, clientcom.info.sessionid);
	var timeoutTimer = null;
	var remoteservice = exports.getService(dataDoc);
	if (!remoteservice) {
		if (!startphase) {
			console.warn("Service not found!", dataDoc);
		} else {
			// execute service call after 2s, the start order of services follows no order
			setTimeout(function() { exports.servicecall(doc, clientcom, true); }, 2000);
		}
		return;
	}
	
	if (remoteservice.com == clientcom) {
		console.warn("Service addressed itself. Not delivering message!",clientcom.info.componentid_);
		return;
	}
	
	if (api.needAck(doc)) {
		timeoutTimer = setTimeout(function() {
			remoteservice.removeListener("ack", receiveAck);
			clientcom.send(api.generateAck(doc, false));
			clientcom = null;
		}, 1500);
		
		function receiveAck(id, response) {
			if (id == doc.requestid_) {
				remoteservice.removeListener("ack", receiveAck);
				clearTimeout(timeoutTimer);
				clientcom.send(api.generateAck(doc, response));
				clientcom = null;
			}
		}
		
		remoteservice.on("ack", receiveAck);
	}
	
	api.setAckRequired(dataDoc, doc.requestid_);
	remoteservice.com.send(dataDoc);
}

exports.service = function(com, id) {
	var that = this;
	that.id = id;
	that.com = com;
	that.info = com.info;
	
	// init service
	that.com.send(api.serviceAPI.init());
	that.com.send(api.serviceAPI.requestProperties());

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
		} else if (api.serviceAPI.isPropertyChange(doc)) {
			properties.change(doc, that.info);
		} else if (api.serviceAPI.isTriggeredEvent(doc)) {
			that.emit("event_triggered", doc);
		} else if (api.serviceAPI.isServiceCall(doc)) {
			exports.servicecall(doc, that.com);
		} else 
			console.log('Unknown type:', doc);
	});
	
	this.abort = function(sceneid) {
		that.com.send(api.serviceAPI.abortExecution(sceneid));
	}
};
require('util').inherits(exports.service, require('events').EventEmitter);