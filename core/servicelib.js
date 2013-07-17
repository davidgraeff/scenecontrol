/**
 * Use this module for services. All necessary communication and protocol work is linked in.
 */

// modules
var path = require('path');

// service main path
exports.servicerootpath = path.dirname(require.main.filename);

// Service ID (derived from directory name)
exports.serviceid = path.basename(exports.servicerootpath);

// command line parameters.
var optimist = require('optimist').usage('Usage: $0	 [options]')
	.options("h",{alias:"help"}).describe("h","Show this help")
	.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
	.options("d",{alias:"debug"}).describe("d","Activate profiling and debug output")
	.options("host",{default:"localhost"}).describe("host","Server Host")
	.options("port",{default:3102}).describe("port","Server port")
	argv = optimist.argv;
	
// Library path
exports.serverpath = path.dirname(__filename);

// Include config from server directory
var configs = require(path.resolve(exports.serverpath,'config.js'));

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

//TODO server communication

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
		// add value store
		m.values[methodtarget] = {};
		// add method
		m[methodname] = function(funKey, funValue) {
			this.values[methodtarget][funKey]=funValue;
		};
	}
	for (var methodname in setKey) {
		var methodtarget = setKey[methodname];
		// add value store
		m.values[methodtarget] = {};
		// add method
		m[methodname] = function(funKey, funValue) {
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