var api = require('./api.js').api;
var services = require('../services.js');
var storage = require('../storage.js');
var properties = require('../properties.js');

exports.clientcom = function(uniqueid) {
	this.name ="No name";
	this.info = null;
	this.state = 1;
	var that = this;
	
	/**
	 * Empty implementation: Have to be overriden by the underlying
	 * socket
	 */
	this.send = function(obj) {}

	this.free = function(obj) {
		if (that.info.provides.indexOf("service")!=-1)
			services.removeService(that.info);
		if (that.info.provides.indexOf("consumer")!=-1)
			properties.removePropertyListener(that.info.sessionid);
	}
	
	this.identified = function(doc) {
		that.info = doc;
		that.info.sessionid = uniqueid;
		that.name = doc.componentid_;
		that.state = 3;
		
		if (that.info.provides.indexOf("service")!=-1)
			services.addService(that, that.info);
		if (that.info.provides.indexOf("consumer")!=-1)
			properties.addPropertyListener(that, that.info.sessionid);
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
				if (api.consumerAPI.isFetchDocuments(doc)) {
					storage.db.collection(doc.type).find(doc.filter).toArray(function(err, items) {
						if (err) {
							return;
						}
						if (items) {
							that.send({type_:"storage",method_:"batch",documents:items});
						}
					});
					return;
				}
				//TODO update, remove docs
				
				that.emit("data", doc, that);
				break;
			default:
				console.warn("State unknown:", that.state);
		}
	}
};

require('util').inherits(exports.clientcom, require('events').EventEmitter);