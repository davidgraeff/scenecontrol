var vows = require('vows'), assert = require('assert');
var controlflow = require('async');

// Test creating scene

// Test importing same scene without overwrite

// Test importing scene with overwrite

// Test adding sceneitem

// Test removing scene

// Test if sceneitem is removed


var storageImporter = require('./storage.import.js');
storageImporter.overwrite = argv.o;
storageImporter.addImportPath(configs.systempaths.path_database_files); // add system import path
if (configs.userpaths.path_database_files) storageImporter.addImportPath(configs.userpaths.path_database_files); // add command line import path if any
if (argv.i) storageImporter.addImportPath(argv.i); // add command line import path if any
if (argv.e) {
	console.log("Only import...");
	storageImporter.importNewFiles(function(err, result) {process.exit(0);});
	return;
}

var startservices = require("./services.processcontroller.js");
var commandsocket = require("./com/socket.js");
var commandwebsocket = require("./com/websocket.js");
var scenes = require("./sceneruntime.js");
var coreservice = require('./core.service.js');
var storage = require('./storage.js');

process.on('exit', function () {
	controlflow.series([scenes.finish, commandwebsocket.finish, commandsocket.finish,coreservice.finish, startservices.finish], function() {
		process.exit(0);
	});
	console.log('Beenden...');
});

// 1) start network socket
// 2) Install missing config files from system and user dir
// 3) start plugin processes (as soon as storage.load() is called)
// 4) Call storage.load
controlflow.series([commandsocket.start, commandwebsocket.start, storage.init, storageImporter.importNewFiles, storage.showstats,
				   startservices.init, coreservice.init, scenes.init], 
	function(err, results){
		if (err) {
			console.error("Error on start: " + err);
			process.exit(1);
		}
	}
);

