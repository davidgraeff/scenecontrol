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
var connects_count = 0;

wsServer.on('request', function(request) {
	var c = request.accept("scenecontrol-protocol", request.origin);
	c.writeDoc = function(doc) {this.sendUTF(JSON.stringify(doc)+"\n");}
	// close socket if no identify message after 1,5s
	var timeoutTimer = setTimeout(function() {c.close();}, 1500);
	wsServer.clients.push(c);
	
	c.com = new clientcom("w"+connects_count++);
	c.com.socket = c;
	c.com.send = function(obj) {
		c.writeDoc(obj);
	}
	
	c.com.once("identified", function() { clearTimeout(timeoutTimer); delete timeoutTimer; });
	c.com.once("failed", function(com) { com.socket.close(); });
	
    c.on('message', function(message) {
        if (message.type === 'utf8') {
			c.com.receive(message.utf8Data);
        }
    });

    c.on('close', function() {
		wsServer.clients.removeElement(c);
		console.log('Client disconnected: '+c.com.name);
		c.com.free();
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
	var q = controlflow.queue(function (task, queuecallback) {
		try {
			task.client.on("close", queuecallback);
			task.client.close();
		} catch(e) {
			console.log("with err", e);
			queuecallback();
		}
	}, 2);
	
	wsServer.clients.forEach(function(client) {
		q.push({client: client});
	});
}