var controlflow = require('async');
var configs = require('../config.js');
var clientcom = require('./clientcom.js').clientcom;
var api = require('./api.js').api;
var rl = require('readline');

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
var connects_count = 0;

function addRawSocket(c) { //'connection' listener
	c.setNoDelay(true);
	c.setEncoding('utf8');
	// close socket if no identify message after 1,5s
	c.setTimeout(1500, function() { c.destroy(); });
	c.writeDoc = function(doc) {this.write(JSON.stringify(doc)+"\n");}
	server.clients.push(c);

	c.com = new clientcom("r"+connects_count++);
	c.com.socket = c;
	c.com.send = function(obj) {
		c.writeDoc(obj);
	}
	
	c.com.once("identified", function(com) { com.socket.setTimeout(0); });
	c.com.once("failed", function(com) { com.socket.destroy(); });
	
	// receive event
	var rlInterface = rl.createInterface(c, c);
	rlInterface.on('line', c.com.receive);
	
	// close event
	c.on('end', function() {
		c.setTimeout(0);
		server.clients.removeElement(c);
		console.log('Client disconnected: '+c.com.name);
		c.com.free();
		delete c.com;
	});
	
	// send identify message
	c.writeDoc(api.methodIdentify("first"));
};

exports.controlport = configs.runtimeconfig.controlport;
exports.serverip = "127.0.0.1"; // ipv4 only?

exports.start = function(callback_out) {
	server.on("error", function() {
		callback("Cannot bind to controlsocket port "+configs.runtimeconfig.controlport,"");
		
	});
	server.on("listening", function() {
		console.log("Controlsocket port: " + configs.runtimeconfig.controlport);
		callback_out();
	});
	server.listen(configs.runtimeconfig.controlport);
}

exports.finish = function(callback_out) {
	server.close(function(err,result){callback_out();});
	
	var q = controlflow.queue(function (task, queuecallback) {
		task.client.on("close", queuecallback);
		task.client.end();
		task.client.destroy();
	}, 2);
	
	server.clients.forEach(function(client) {
		q.push({client: client});
	});
}