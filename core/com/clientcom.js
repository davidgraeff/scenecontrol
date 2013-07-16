var api = require('./api.js').api;
var services = require('../services.js');
var propertyListener = require('../properties.listener.js');
var storageListener = require('../storage.listener.js');
var assert = require('assert');

exports.clientcom = function(uniqueid, socket) {
	// Public API
	this.name ="No name";
	this.remoteAddress = "";
	this.info = null;
	this.isservice = false;
	this.state = 1;
	this.socket = socket; 
	// Private
	var that = this;
	
	assert(uniqueid, "Unique ID of clientcom not set!");
	
	/**
	 * Empty implementation: Have to be overriden by the underlying
	 * socket
	 */
	this.send = function(obj) {}

	this.free = function(obj) {
		that.socket = null; // remove socket reference
		if (!that.info.provides)
			return;
		if (that.info.provides.indexOf("service")!=-1)
			services.removeService(that.info);
		if (that.info.provides.indexOf("consumer")!=-1)
			propertyListener.removeListener(that.info.sessionid);
		if (that.info.provides.indexOf("manipulator")!=-1)
			storageListener.removeListener(that.info.sessionid);
	}
	
	this.identified = function(doc) {
		that.info = doc;
		that.info.sessionid = uniqueid;
		that.name = doc.componentid_;
		that.state = 3;
		if (!that.info.provides) {
			console.warn("Identify: Missing provides", that.info);
			return;
		}
		
		if (that.info.provides.indexOf("service")!=-1) {
			services.addService(that, that.info);
			that.isservice = true;
		}
		if (that.info.provides.indexOf("consumer")!=-1) {
			propertyListener.addListener(that, that.info.sessionid);
			that.isservice = true;
		}
		if (that.info.provides.indexOf("manipulator")!=-1) {
			storageListener.addListener(that, that.info.sessionid);
			that.isservice = true;
		}
	};
	
	this.receive = function(data) {
		var doc;
		try {
			if (typeof data == "object")
				doc = data;
			else
				doc = JSON.parse(data);
		} catch(e) {
			console.error("Parsing failed:", e,data);
			return;
		}
		
		// expect ack as first and identity as second message
		switch (that.state) {
			case 1:
				if (!api.isAck(doc, "first")) {
					that.emit("failed", that);
					return;
				}
				that.state = 2;
				break;
			case 2:
				// check for identity
				if (doc.method_ != "identify") {
					that.emit("failed", that);
					return;
				}
				
				if (api.needAck(doc))
					that.send(api.generateAck(doc));
				
				that.emit("identified", that);

				that.identified(doc);
				break;
			case 3:
				/* A service call is either for information exchange
				 * between services (service<-->service communication) or
				 * from other kind of clients which want to call a function
				 * of a service. */
				if (api.isExecuteCall(doc)) {
					services.servicecall(doc, that);
					return;
				}
				that.emit("data", doc, that);
				break;
			default:
				console.warn("State unknown:", that.state);
		}
	}
};

require('util').inherits(exports.clientcom, require('events').EventEmitter);