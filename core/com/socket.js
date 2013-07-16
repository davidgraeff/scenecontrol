var controlflow = require('async');
var configs = require('../config.js');
var clientcom = require('./clientcom.js').clientcom;
var api = require('./api.js').api;
var rl = require('readline');
var fs = require('fs');

Array.prototype.removeElement = function(elem) { this.splice(this.indexOf(elem), 1); }

function readKey() {
	if (fs.existsSync(configs.userpaths.path_server_cert+'/server.key'))
		return fs.readFileSync(configs.userpaths.path_server_cert+'/server.key');
	if (fs.existsSync(configs.systempaths.path_server_cert+'/server.key'))
		return fs.readFileSync(configs.systempaths.path_server_cert+'/server.key');
	
	console.error("No Server socket ssl key file found: "+configs.systempaths.path_server_cert+'/server.key');
	process.exit(1);
}

function readCertificate() {
	if (fs.existsSync(configs.userpaths.path_server_cert+'/server.crt'))
		return fs.readFileSync(configs.userpaths.path_server_cert+'/server.crt');
	if (fs.existsSync(configs.systempaths.path_server_cert+'/server.crt'))
		return fs.readFileSync(configs.systempaths.path_server_cert+'/server.crt');
	
	console.error("No Server socket ssl crt file found: "+configs.systempaths.path_server_cert+'/server.crt');
	process.exit(1);
}

var options = {
	key: readKey(),
	cert: readCertificate(),
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

	c.com = new clientcom("r"+connects_count++, c);
	// Override send method and remoteAddress
	c.com.send = function(obj) {
		c.writeDoc(obj);
	}
	c.com.remoteAddress = c.remoteAddress;
	
	c.com.once("identified", function(com) { com.socket.setTimeout(0); });
	c.com.once("failed", function(com) {
		console.log('Client failed: '+ (c.com.isservice ? c.com.name : c.com.remoteAddress));
		com.socket.destroy();
	});
	
	// receive event
	var rlInterface = rl.createInterface(c, c);
	rlInterface.on('line', c.com.receive);
	
	// close event
	c.on('end', function() {
		c.setTimeout(0);
		server.clients.removeElement(c);
		console.log('Client disconnected: '+ (c.com.isservice ? c.com.name : c.com.remoteAddress));
		c.com.free();
		delete c.com;
	});
	
	// send identify message
	console.log('Client connected. Wait for identity: '+c.com.remoteAddress);
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
		try {
			task.client.on("close", queuecallback);
			task.client.end();
			task.client.destroy();
		} catch(e) {
			console.log("with err", e);
			queuecallback();
		}
	}, 2);
	
	server.clients.forEach(function(client) {
		q.push({client: client});
	});
	
	
}