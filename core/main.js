// control flow
var controlflow = require('async');

// Print usage information, look at command line parameters
var configs = require('./config.js');
var optimist = require('optimist').usage('Usage: $0	 [options]')
	.options("h",{alias:"help"}).describe("h","Show this help")
	.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
	.options("i",{alias:"import"}).describe("i","Import json files from path")
	.options("o",{alias:"override"}).describe("o","Override existing documents when importing files with the same id+type")
	.options("e",{alias:"exit"}).describe("e","Do not enter the mainloop and exit after importing")
	.options("dbname",{default:configs.runtimeconfig.databasename}).describe("dbname","Database name")
	.options("cport",{default:configs.runtimeconfig.controlport}).describe("cport","TCP Controlsocket port")
	.options("wport",{default:configs.runtimeconfig.websocketport}).describe("wport","Websocket Controlsocket port"),
	argv = optimist.argv;
	
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


// install new database files
var storage = require('./storage.js');
storage.overwrite = argv.o;
storage.addImportPath(configs.systempaths.path_database_files); // add system import path
if (configs.userpaths.path_database_files) storage.addImportPath(configs.userpaths.path_database_files); // add command line import path if any
if (argv.i) storage.addImportPath(argv.i); // add command line import path if any
if (argv.e) {
	console.log("Only import...");
	storage.importNewFiles(function(err, result) {process.exit(0);});
	return;
}

var startservices = require("./startservices.js");
var commandsocket = require("./com/socket.js");
var commandwebsocket = require("./com/websocket.js");
var scenes = require("./scenes.js");

// exit handling
process.stdin.resume();
process.on('SIGINT', function () {
	controlflow.series([scenes.finish, commandwebsocket.finish, commandsocket.finish, startservices.finish], function() {
		process.exit(0);
	});
});
process.on('exit', function () {
	console.log('Beenden...');
});

// 1) start network socket
// 2) Install missing config files from system and user dir
// 3) start plugin processes (as soon as storage.load() is called)
// 4) Call storage.load
controlflow.series([commandsocket.start, commandwebsocket.start, storage.init, storage.importNewFiles, storage.showstats,
				   startservices.init, scenes.init], 
	function(err, results){
		if (err) {
			console.error("Error on start: " + err);
			process.exit(1);
		}
	}
);


/// Tests
setTimeout(function() {
	console.log("Test...");
	var scene = scenes.getScene("54114e2456df4aa2a15b161a47a3d8d1");
	console.log("Execute: "+scene.scene.name);
	scene.startScene();
}, 2000);