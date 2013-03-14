var api = require('./api.js').api;
var services = require('../services.js');

exports.clientcom = function(socket) {
	this.socket = socket;
	this.name ="No name";
	this.info = null;
	this.state = 1;
	var that = this;
	
	this.send = function(obj) {
		socket.writeDoc(obj);
	}
	
	this.receive = function(rawString) {
		var doc;
		try {
			doc = JSON.parse(rawString);
		} catch(e) {
			console.error("Parsing failed:", e,rawString);
			return;
		}
		
		// expect ack as first and identity as second message
		switch (that.state) {
			case 1:
				if (!api.isAck(doc, "first")) {
					socket.destroy();
					return;
				}
				that.state = 2;
				break;
			case 2:
				// check for identity
				if (doc.method_ != "identify") {
					socket.destroy();
					return;
				}
				
				that.emit("identified", that.socket);

				that.info = doc;
				that.name = doc.componentid_;
				that.state = 3;

				if (api.needAck(doc))
					socket.writeDoc(api.generateAck(doc));
				
				if (doc.provides.indexOf("service")!=-1)
					services.addService(that);
				
				break;
			case 3:
				if (api.needAck(doc))
					socket.writeDoc(api.generateAck(doc));
				
				that.emit("data", doc);
				break;
			default:
				console.warn("State unknown:", that.state);
		}
	}
};

require('util').inherits(exports.clientcom, require('events').EventEmitter);