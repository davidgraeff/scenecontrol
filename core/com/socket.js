var net = require('net');
var tls = require('tls');
var fs = require('fs');
var controlflow = require('async');
var configs = require('../config.js');
var clientcom = require('./clientcom.js');
var plugincom = require('./plugincom.js');

var options = {
  key: fs.readFileSync('certificates/server.key'),
  cert: fs.readFileSync('certificates/server.crt'),
  passphrase: "1234",
  // This is necessary only if using the client certificate authentication.
  requestCert: true/*,

  // This is necessary only if the client uses the self-signed certificate.
  ca: [ fs.readFileSync('certificates/client.crt') ]*/
};

var server = tls.createServer(options, function(c) { //'connection' listener
	c.setNoDelay(true);
	//c.setKeepAlive(true);
	console.log('service connected');
	c.setEncoding('utf8');
	c.on('data', function(data) {
		try {
			var obj = JSON.parse(data);
			console.log('service data', obj);
			if (obj.type_!="ack")
				c.write(JSON.stringify({"type_":"ack","responseid_":obj.requestid_}));
		} catch(e) {
			
		}
	});
	c.on('end', function() {
		console.log('service disconnected');
	});
	c.write(JSON.stringify({"type_":"auth","method_":"identify","apiversion":10,"provides":"core","requestid_":"bla"}));
// 	c.pipe(c);
});

//////////////////////////////////////////////////////
var WebSocketServer = require('websocket').server;
var http = require('http');

var httpserver = http.createServer(function(request, response) {});

// create the server
wsServer = new WebSocketServer({
    httpServer: server
});

// WebSocket server
wsServer.on('request', function(request) {
    var connection = request.accept(null, request.origin);

    // This is the most important callback for us, we'll handle
    // all messages from users here.
    connection.on('message', function(message) {
        if (message.type === 'utf8') {
            // process WebSocket message
        }
    });

    connection.on('close', function(connection) {
        // close user connection
    });
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
