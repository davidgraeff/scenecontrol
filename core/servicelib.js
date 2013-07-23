/**
 * Use this module for services. All necessary communication and protocol work is linked in.
 */

/**
* Service global variables
*/
var path = require('path');
exports.servicerootpath = path.dirname(require.main.filename); // service main path
exports.serviceid = path.basename(exports.servicerootpath); // Service ID (derived from directory name)
exports.serverpath = path.dirname(__filename); // Server path
// Include config from server directory
var configs = require(path.resolve(exports.serverpath,'config.js'));

exports.init = function(optimist) {
	// command line parameters.
	if (!optimist)
		optimist = require('optimist').usage('Usage: $0	 [options]');

	optimist.options("h",{alias:"help"}).describe("h","Show this help")
			.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
			.options("d",{alias:"debug"}).describe("d","Activate profiling and debug output")
			.options("host",{default:"localhost"}).describe("host","Server Host")
			.options("port",{default:3102}).describe("port","Server port")
			.options("instanceid",{demand:true}).describe("instanceid","Instance ID to separate from other instances");
			
	argv = optimist.argv;

	/**
	* Service global variables
	*/
	exports.instanceid = argv.instanceid; // Instance ID
	exports.serverhost = argv.host;
	exports.serverport = argv.port;
	
	// Logging
	require(path.resolve(exports.serverpath,'logging.js')).init(exports.serviceid+":"+exports.instanceid);

	// Show Help or Version info?
	if (argv.help) {
		console.log(configs.aboutconfig.ABOUT_SERVICENAME+" "+configs.aboutconfig.ABOUT_VERSION);
		console.log("Please visit http://davidgraeff.github.com/scenecontrol for more information");
		console.log(optimist.help());
		process.exit(0);
	} else if (argv.version) {
		console.log(JSON.stringify(configs.aboutconfig));
		process.exit(0);
	}

	// exit handling
	process.stdin.resume();
	process.on('SIGINT', function () {
		process.exit(0);
	});
}

exports.initCommunication = function(certPassphrase) {
	//server communication
	var socketCert = require(path.resolve(exports.serverpath,'com/socket.certificate.js'));
	var tls = require('tls'),
		fs = require('fs');

	var options = {
		key: socketCert.readFromCertificatePaths([exports.serviceid+".key","generic.key"], [configs.systempaths.path_certs_clients,configs.userpaths.path_certs_clients]),
		cert: socketCert.readFromCertificatePaths([exports.serviceid+".crt","generic.crt"], [configs.systempaths.path_certs_clients,configs.userpaths.path_certs_clients]),
		ca: socketCert.readAllowedCertificates([configs.userpaths.path_cert_server, configs.systempaths.path_cert_server]),
		passphrase:certPassphrase,
		host:exports.serverhost,
		port:exports.serverport,
		rejectUnauthorized: false
	};

	var rl = require('readline');
	var conn = tls.connect(options, function(c) {
	if (conn.authorized) {
		console.log("Connection authorized by a Certificate Authority.");
	} else {
		console.log("Connection not authorized: " + conn.authorizationError)
	}
		conn.setNoDelay(true);
		conn.setEncoding('utf8');
		// close socket if no identify message after 1,5s
		conn.setTimeout(1500, function() { conn.destroy(); });
		conn.writeDoc = function(doc) {this.write(JSON.stringify(doc)+"\n");}
		
		// receive event
		receive_doc_abstract = protocol_identify;
		var rlInterface = rl.createInterface(conn, conn);
		rlInterface.on('line', receive_data);
	});
	// close event
	conn.on('end', function() {
		conn.setTimeout(0);
		console.error("Server diconnected");
		process.exit(1);
	});
	// close event
	conn.on('error', function(err) {
		console.error("Server diconnected", err);
		process.exit(1);
	});


	function receive_data(data) {
			var doc;
			try {
				if (typeof data == "object")
					doc = data;
				else
					doc = JSON.parse(data);
			} catch(e) {
				console.error("Parsing failed:", e,data);
				return;
			}
			
			receive_doc_abstract(doc);
	}

	var api = require(path.resolve(exports.serverpath,'com/api.js')).api;

	function receive_doc_abstract(doc) {}

	// EventEmitter
	var Eventer = function(){};
	require('util').inherits(Eventer, require('events').EventEmitter);
	exports.receiver=new Eventer();

	function receive_doc(doc) {
		if (api.needAck(doc))
			conn.writeDoc(api.generateAck(doc));
// 		console.log("RECEIVED", doc);
		
		if (doc.method_=="initialize")
			exports.ready();
		else if (doc.method_=="requestProperties") {
			for (var model in exports.models) {
				//TODO server message

			}
		} else if (doc.method_=="instanceConfiguration")
			if (doc.data) exports.gotconfiguration(doc.data);
		else if (exports.methods && exports.methods[doc.method_])
			exports.methods[doc.method_](doc);
		else
			console.warn("Method not found:", doc.method_);
	}

	function protocol_identify(doc) {
		var requestid = "service_ident";
		if (api.needAck(doc))
			conn.writeDoc(api.generateAck(doc));
		if (doc.method_=="identify") {
			conn.writeDoc({"method_":"identify","apiversion":10,"provides":["service","consumer","manipulator"],"requestid_":requestid, componentid_:exports.serviceid, instanceid_:exports.instanceid});
			return;
		}
		if (api.isAck(doc, requestid)) {
			// Identification complete, disable timeout and set regular receive method
			conn.setTimeout(0);
			receive_doc_abstract = receive_doc;
			return;
		}
		conn.close();
	}
}

//TODO getProperty API (models)

//TODO
exports.respond = function(request) {return null;}

// Placeholder functions and objects
exports.models = {};
exports.methods = null;
exports.ready = function() {}
exports.gotconfiguration = function() {}

exports.addModel = function(id, key, getKey, setKey) {
	var m = {id:id,key:key,values:{}};
	for (var methodname in getKey) {
		var methodtarget = getKey[methodname];
		if (!methodtarget) {
			console.warn("Ignore addModel getKey:", getKey, methodname);
			continue;
		}
		// add value store
		m.values[methodtarget] = {};
		// add method
		m[methodname] = function(funKey, funValue) {
			this.values[methodtarget][funKey]=funValue;
		};
	}
	for (var methodname in setKey) {
		var methodtarget = setKey[methodname];
		if (!methodtarget) {
			console.warn("Ignore addModel setKey:", setKey, methodname);
			continue;
		}
		// add method
		m[methodname] = function(funKey, funValue) {
			//console.warn("setkey:", this);
			
			if (!this.values[methodtarget])
				this.values[methodtarget] = {};

 			this.values[methodtarget][funKey]=funValue;
			//TODO server message
		};
	}
	exports.models[id] = m;
}

exports.resetModels = function() {
	for (var model in exports.models) {
		//TODO server message
		
		// delete
		exports.models[model].values = {};
	}
}