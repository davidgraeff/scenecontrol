var configs = require('./config.js');
var child_process = require('child_process');
var storage = require('./storage.js');
var commandsocket = require("./com/socket.js");
var childs = [];

exports.finish = function(callback) {
	childs.forEach(function(child) {
		child.kill();
	});
	callback();
}

exports.init = function(callback) {
	console.log('Autostarting plugin processes...');
	storage.db.collection('configuration').find().toArray(function(err, items) {
		if (err) {
			console.warn("Could not get configurations for services ", err);
		}
		if (items) {
			items.forEach(function(config) {
				if (!config.autostart) 
					return;
				console.log("Start service '"+ config.componentid_ + "' with configuration: "+ config.instanceid_);
				var child = child_process.spawn(configs.systempaths.path_plugins+"/"+config.componentid_,[config.instanceid_,commandsocket.serverip,commandsocket.controlport]);
				child.name = config.componentid_;
				child.instance = config.instanceid_;
				childs.push(child);
				child.stdout.on('data', function (data) {
					console.log(data.toString("utf8", 0, data.length-1));
				});

				child.stderr.on('data', function (data) {
					console.error(data.toString("utf8", 0, data.length-1));
				});

				child.on('exit', function (code) {
					console.log('Service finished: ' + this.name + " ("+ this.instance +")");
					var i = childs.indexOf(this);
					if (i!=-1)
						childs = childs.splice(i,1);
				});
			});
		}

		callback(null, null);
	});
}