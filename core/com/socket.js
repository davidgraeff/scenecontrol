var controlflow = require('async');
var configs = require('../config.js');
var clientcom = require('./clientcom.js').clientcom;
var api = require('./api.js').api;
var socketCert = require('./socket.certificate.js');
var rl = require('readline');
var fs = require('fs');

Array.prototype.removeElement = function(elem) { this.splice(this.indexOf(elem), 1); }

var server;
var connects_count = 0;
function addRawSocket(c) { //'connection' listener
	if (!c.authorized) {
		console.log("Connection "+c.remoteAddress+" not authorized: " + c.authorizationError);
		c.end();
		return;
	}
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
	c.com.once("failed", function(com, reason) {
		console.log('Client failed: '+ (c.com.isservice ? c.com.name : c.com.remoteAddress) + "; "+reason);
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
	//console.log('Client connected. Wait for identity: '+c.com.remoteAddress);
	c.writeDoc(api.methodIdentify("first"));
};

exports.controlport = configs.runtimeconfig.controlport;
exports.serverip = "127.0.0.1"; // ipv4 only?

exports.start = function(callback_out) {
	var options = {
		key: socketCert.readFromCertificatePaths(['server.key'], [configs.systempaths.path_cert_server,configs.userpaths.path_cert_server]),
		cert: socketCert.readFromCertificatePaths(['server.crt'], [configs.systempaths.path_cert_server,configs.userpaths.path_cert_server]),
		ca: socketCert.readAllowedCertificates([configs.userpaths.path_certs_clients, configs.systempaths.path_certs_clients]),
		passphrase: configs.runtimeconfig.sslcertificatekey,
		// This is necessary only if using the client certificate authentication.
		requestCert: true,
		rejectUnauthorized: false, // we reject ourselfs
		handshakeTimeout: 10
	};
	/**
	* Create listener socket with above options
	*/
	server = require('tls').createServer(options, addRawSocket);
	server.clients = [];
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