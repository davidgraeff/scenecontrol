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
var fs = require('fs');
var childs = {};

var controller = {
	startservice: function(config) {
		var configid = config.componentid_+"."+config.instanceid_;
		if (!config.autostart || childs[configid]) 
			return;
		if (!fs.existsSync(configs.systempaths.path_plugins+"/"+config.componentid_))
			return;
		var child = child_process.spawn(configs.systempaths.path_plugins+"/"+config.componentid_,[config.instanceid_,commandsocket.serverip,commandsocket.controlport]);
		child.name = config.componentid_;
		child.instance = config.instanceid_;
		childs[configid] = child;
		child.stdout.on('data', function (data) {
			console.log(data.toString("utf8", 0, data.length-1));
		});
		
		child.stderr.on('data', function (data) {
			console.error(data.toString("utf8", 0, data.length-1));
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