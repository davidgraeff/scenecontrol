var net = require('net');
var tls = require('tls');
var fs = require('fs');
var controlflow = require('async');
var configs = require('../config.js');

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
  console.log('server connected');
  c.setEncoding('utf8');
  c.on('end', function() {
    console.log('server disconnected');
  });
  c.write('hello\n');
  c.pipe(c);
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
