require('./logging.js').init("Core");

// Print usage information, command line parameters
var configs = require('./config.js');
var optimist = require('optimist').usage('Usage: $0	 [options]')
	.options("h",{alias:"help"}).describe("h","Show this help")
	.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
	.options("d",{alias:"debug"}).describe("d","Activate profiling and debug output")
	.options("i",{alias:"import"}).describe("i","Import json files from path")
	.options("o",{alias:"override"}).describe("o","Override existing documents when importing files with the same id+type")
	.options("e",{alias:"exit"}).describe("e","Do not enter the mainloop and exit after init")
	.options("drop",{}).describe("drop","Drop database content. Warning! This can't be undone! Only works with additional -e flag.")
	.options("dbname",{default:configs.runtimeconfig.databasename}).describe("dbname","Database name")
	.options("cport",{default:configs.runtimeconfig.controlport}).describe("cport","TCP Controlsocket port")
	.options("wport",{default:configs.runtimeconfig.websocketport}).describe("wport","Websocket Controlsocket port"),
	argv = optimist.argv;
	
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

// Print app name
console.log(configs.aboutconfig.ABOUT_SERVICENAME+" "+configs.aboutconfig.ABOUT_VERSION);


// Storage: add import paths
var storageImporter = require('./storage.import.js');
var storage = require('./storage.js');

// Drop database?
if (argv.drop) {
	console.log("Dropping database!");
	storage.init(function(err) {
		if (!err)
			storage.drop();
	});
	return;
}

storageImporter.overwrite = argv.o;
storageImporter.addImportPath(configs.systempaths.path_database_files); // add system import path
if (configs.userpaths.path_database_files) storageImporter.addImportPath(configs.userpaths.path_database_files); // add command line import path if any
if (argv.i) storageImporter.addImportPath(argv.i); // add command line import path if any

// Only import: install new database files and exit
if (argv.e) {
	console.log("Only import...");
	storage.init(function(err) {
		if (!err)
			storageImporter.importNewFiles(function(err, result) {process.exit(0);});
	});
	return;
}

// init modules
var controlflow = require('async');
var startservices = require("./services.processcontroller.js");
var commandsocket = require("./com/socket.js");
var commandwebsocket = require("./com/websocket.js");
var scenes = require("./sceneruntime.js");
var coreservice = require('./core.service.js');

// exit handling
process.stdin.resume();
process.on('SIGINT', function () {
	controlflow.series([scenes.finish, commandwebsocket.finish, commandsocket.finish,coreservice.finish, startservices.finish, storage.finish], function() {
		process.exit(0);
	});
});
process.on('exit', function () {
	console.log('Beenden...');
});

// 1) start network sockets
// 2) init storage
// 3) Install missing config files from system and user dir
// 4) start plugin processes (as soon as storage.load() is called)
// 5) Start core service
// 6) Start scenes (Register events to services)

controlflow.series([commandsocket.start, commandwebsocket.start, storage.init, storageImporter.importNewFiles, storage.showstats,
				   coreservice.init, startservices.init, scenes.init], 
	function(err, results){
		if (err) {
			console.error("Error on start: " + err);
			process.exit(1);
		}
	}
);