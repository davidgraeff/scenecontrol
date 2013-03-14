var controlflow = require('async');
var configs = require('../config.js');
var clientcom = require('./clientcom.js').clientcom;
var api = require('./api.js').api;

Array.prototype.removeElement = function(elem) { this.splice(this.indexOf(elem), 1); }

exports.websocketport = configs.runtimeconfig.websocketport;
exports.serverip = "127.0.0.1"; // ipv4 only?

var httpserver = require('http').createServer(function(request, response) {});
var WebSocketServer = require('websocket').server;
var wsServer = new WebSocketServer({ httpServer: httpserver});
wsServer.clients = [];

wsServer.on('request', function(request) {
    var c = request.accept(null, request.origin);
	c.writeDoc = function(doc) {this.write(JSON.stringify(doc)+"\n");}
	// close socket if no identify message after 1,5s
	c.setTimeout(1500, c.destroy);
	clients.push(c);
	
	c.com = new clientcom(c);
	
	c.com.on("identified", function() { c.setTimeout(0); });
	
    // This is the most important callback for us, we'll handle
    // all messages from users here.
    c.on('message', function(message) {
        if (message.type === 'utf8') {
			var remoteDoc;
			try {
				remoteDoc = JSON.parse(message.data);
			} catch(e) {
				wsServer.clients.removeElement(c);
				c.destroy();
				return;
			}
			
			c.com.receive(remoteDoc);
        }
    });

    c.on('close', function(c) {
		wsServer.clients.removeElement(c);
		console.log('Client disconnected: '+c.com.name);
		delete c.com;
    });
	
	// send identify message
	c.writeDoc(api.methodIdentify("first"));
});


exports.start = function(callback_out) {
	httpserver.on("error", function() {
		callback("Cannot bind to Websocket port "+configs.runtimeconfig.websocketport,"");
		
	});
	httpserver.on("listening", function() {
		console.log("Websocket port: " + configs.runtimeconfig.websocketport);
		callback_out();
	});
	httpserver.listen(configs.runtimeconfig.websocketport);
}

exports.finish = function(callback_out) {
	httpserver.close(function(err,result){callback_out();});
}