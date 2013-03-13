var controlflow = require('async');
var configs = require('../config.js');
var clientcom = require('./clientcom.js').clientcom;
var api = require('./api.js').api;

Array.prototype.removeElement = function(elem) { this.splice(this.indexOf(elem), 1); }


var fs = require('fs');
var options = {
  key: fs.readFileSync('certificates/server.key'),
  cert: fs.readFileSync('certificates/server.crt'),
  passphrase: "1234",
  // This is necessary only if using the client certificate authentication.
  requestCert: true
};
var server = require('tls').createServer(options, addRawSocket);
server.clients = [];

function addRawSocket(c) { //'connection' listener
	c.setNoDelay(true);
	c.setEncoding('utf8');
	// close socket if no identify message after 1,5s
	c.setTimeout(1500, function() { c.destroy(); });
	c.writeDoc = function(doc) {this.write(JSON.stringify(doc)+"\n");}
	server.clients.push(c);
	
	c.com = new clientcom(c);
	
	c.com.on("identified", function(socket) { socket.setTimeout(0); });
	
	// receive event
	c.on('data',c.com.receive);
	
	// close event
	c.on('end', function() {
		c.setTimeout(0);
		server.clients.removeElement(c);
		console.log('Client disconnected: '+c.com.name);
		delete c.com;
	});
	
	// send identify message
	c.writeDoc(api.methodIdentify("first"));
};

//////////////////////////////////////////////////////
var httpserver = require('http').createServer(function(request, response) {});
var WebSocketServer = require('websocket').server;
var wsServer = new WebSocketServer({ httpServer: httpserver});
wsServer.clients = [];
// WebSocket server
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

exports.controlport = configs.runtimeconfig.controlport;
exports.websocketport = configs.runtimeconfig.websocketport;
exports.serverip = "127.0.0.1"; // ipv4 only?

exports.start = function(callback_out) {
	controlflow.series([
		function(callback){
			server.on("error", function() {
				callback("Cannot bind to controlsocket port "+configs.runtimeconfig.controlport,"");
				
			});
			server.on("listening", function() {
				console.log("Controlsocket port: " + configs.runtimeconfig.controlport);
				callback(null,"");
			});
			server.listen(configs.runtimeconfig.controlport);
		},
		function(callback){
			httpserver.on("error", function() {
				callback("Cannot bind to Websocket port "+configs.runtimeconfig.websocketport,"");
				
			});
			httpserver.on("listening", function() {
				console.log("Websocket port: " + configs.runtimeconfig.websocketport);
				callback(null,"");
			});
			httpserver.listen(configs.runtimeconfig.websocketport);
		}
	],
	callback_out);
}

exports.finish = function(callback_out) {
	controlflow.parallel([
			function(callback) {
				server.close(function(err,result){callback();});
				
				var q = controlflow.queue(function (task, queuecallback) {
					task.client.on("close", queuecallback);
					task.client.end();
					task.client.destroy();
				}, 2);
				
				server.clients.forEach(function(client) {
					q.push({client: client});
				});
			},
			function(callback) {
				httpserver.close(function(err,result){callback();});
			},
		], callback_out);
}