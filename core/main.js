// control flow
var controlflow = require('async');

// Print usage information, look at command line parameters
var configs = require('./config.js');
var optimist = require('optimist').usage('Usage: $0	 [options]')
	.options("h",{alias:"help"}).describe("h","Show this help")
	.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
	.options("i",{alias:"import"}).describe("i","Import json files from path")
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


var storage = require('./storage.js');
var autostartplugins = require("./startplugins.js");
var commandsocket = require("./com/socket.js");

// install new database files
storage.addImportPath(configs.runtimeconfig.ROOM_DATABASEIMPORTPATH); // add system import path
if (argv.i) storage.addImportPath(argv.i); // add command line import path if any
if (argv.e) {
	console.log("Only import...");
	storage.importNewFiles(function(err, result) {process.exit(0);});
}

// exit handling
process.stdin.resume();
process.on('SIGINT', function () {
	process.exit(0);
});
process.on('exit', function () {
	console.log('Beenden...');
});

// 1) start network socket
// 2) Install missing config files from system and user dir
// 3) start plugin processes (as soon as storage.load() is called)
// 4) Call storage.load
controlflow.series([commandsocket.start, storage.init, storage.importNewFiles, 
				   autostartplugins.init, storage.load], 
	function(err, results){
		if (err) {
			console.error("Error on start: " + err);
			process.exit(1);
		}
	}
);
