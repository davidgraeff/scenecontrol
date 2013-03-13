var api = require('./api.js').api;

exports.clientcom = function(socket, id) {
	this.socket = socket;
	this.name ="No name";
	this.info = null;
	this.sessionid = id;
	this.state = 1;
	var that = this;
	
	this.receive = function(rawString) {
		var doc;
		try {
			doc = JSON.parse(rawString);
		} catch(e) {
			console.error("Parsing failed:", e,data.length);
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
				//TODO emit for every "provides": eg emit "provide_service"
				
				console.log('New client "'+doc.componentid_+'". Provides: ',doc.provides);
				that.info = doc;
				that.name = doc.componentid_;
				that.state = 3;
				
				if (api.needAck(doc))
					socket.writeDoc(api.generateAck(doc));
				
				socket.writeDoc(api.serviceAPI.init());
				
				break;
			case 3:
				if (api.isAck(doc))
					return;
				
				if (api.needAck(doc))
					socket.writeDoc(api.generateAck(doc));
				
				if (api.isForStorage(doc)) {
					that.emit("modify_doc", doc, that.sessionid);
				} else if (api.isServiceCall(doc)) {
					that.emit("service_call", api.getDataFromClientCall(doc, that.sessionid));
				} else 
					console.log('Unknown type:', doc);
				break;
			default:
				console.warn("State unknown:", that.state);
		}
	}
};

require('util').inherits(exports.clientcom, require('events').EventEmitter);