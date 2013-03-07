var net = require('net');
var tls = require('tls');
var fs = require('fs');

var options = {
  key: fs.readFileSync('certificates/server-key.key'),
  cert: fs.readFileSync('certificates/server-cert.crt'),

  // This is necessary only if using the client certificate authentication.
  requestCert: true,

  // This is necessary only if the client uses the self-signed certificate.
  ca: [ fs.readFileSync('client-cert.pem') ]
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
server.listen(3101, function() { //'listening' listener
  console.log('server bound');
}); 

//////////////////////////////////////////////////////
var WebSocketServer = require('websocket').server;
var http = require('http');

var httpserver = http.createServer(function(request, response) {
    // process HTTP request. Since we're writing just WebSockets server
    // we don't have to implement anything.
});
httpserver.listen(3102, function() { });

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