/**
 * This module is for starting/killing of service processes on start/exit
 * as well as on dynamic config-changes.
 */
var configs = require('./config.js');
var child_process = require('child_process');
var storage = require('./storage.js');
var commandsocket = require("./com/socket.js");
var storageListener = require('./storage.listener.js');
var api = require('./com/api.js').api;
var logging = require('./logging.js');
var fs = require('fs');
var childs = {};

/**
 * Private Controller object. This is used by the public init
 * method to start all processes, that have a configuration stored.
 */
var controller = {
	/**
	 * Start process that belongs to the "config" configuration.
	 */
	startservice: function(config) {
		// Determine if we already have started this process and if
		// autostarting the process is allowed.
		var configid = config.componentid_+"."+config.instanceid_;
		if (!config.autostart || childs[configid]) 
			return;
		// Determine if executable (or directory) with install_path/component_name exists.
		var targetexecutable = configs.systempaths.path_plugins+"/"+config.componentid_;
		if (!fs.existsSync(targetexecutable))
			return;
		
		var child; // the child process variable
		
		// If targetexecutable is a directory, enter that one and determine if an index.js file
		// exists: This indicates a node-js based service.
		if (fs.statSync(targetexecutable).isDirectory()) {
			targetexecutable += "/index.js";
			if (!fs.existsSync(targetexecutable))
				return;
			// Add index.js to targetexecutable and put that as first argument
			var proc_arguments = [targetexecutable, "--instanceid",config.instanceid_,"--host",commandsocket.serverip,"--port",commandsocket.controlport];
			//require('process').execPath: we want to execute the node binary
			child = child_process.spawn(process.execPath,proc_arguments);
		} else
			// We always have to provide three arguments: Instance ID, Server IP, Server Port
			child = child_process.spawn(targetexecutable,[config.instanceid_,commandsocket.serverip,commandsocket.controlport]);
		
		// Add some identification attributes and add this process to the process array.
		child.name = config.componentid_;
		child.instance = config.instanceid_;
		childs[configid] = child;
		
		// Connect to stdout and stderr.
		child.stdout.on('data', function (data) {
			logging.origLog(data.toString("utf8", 0, data.length-1));
		});
		
		child.stderr.on('data', function (data) {
			logging.origErr(data.toString("utf8", 0, data.length-1));
		});
		
		child.on('exit', function (code) {
			if (code!=0) {
				console.warn("Start service failed '"+ config.componentid_ + "' with configuration: "+ config.instanceid_);
			} else
				console.log('Service finished: ' + this.name + " ("+ this.instance +")");
			delete childs[configid];
		});
	},
	send: function(storageModified) {
		var config = api.manipulatorAPI.extractDocument(storageModified);
		if (config.type_ != 'configuration')
			return;
		
		var configid = config.componentid_+"."+config.instanceid_;
		
		if (childs[configid]) {
			// kill child
			childs[configid].kill();
			delete childs[configid];
		}
		
		// start instance (again) after 1s
		if (api.manipulatorAPI.isDocumentUpdate(storageModified)) {
			setTimeout(function() { controller.startservice(config); }, 1000);
		}
	}
}

exports.finish = function(callback) {
	storageListener.removeListener(controller);
	for (var i in childs)
		childs[i].kill();
	childs = {};
	callback();
}

exports.init = function(callback) {
	storageListener.addListener(controller, "service.processcontroller");
	storage.getDocuments('configuration', {}, function(err, items) {
		if (err) {
			console.warn("Could not get configurations for services ", err);
		}
		if (items) {
			console.log('Autostarting service processes: '+items.length);
			items.forEach(function(config) {
				controller.startservice(config);
			});
		}

		callback(null, null);
	});
}