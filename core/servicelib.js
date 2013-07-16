/**
 * Use this module for services. All necessary communication and protocol work is linked in.
 */

// modules
var path = require('path');

// service main path
exports.servicerootpath = require('path').dirname(require.main.filename);

// Service ID (derived from directory name)
exports.serviceid = require('path').basename(exports.servicerootpath);

// command line parameters. We
var optimist = require('optimist').usage('Usage: $0	 [options]')
	.options("h",{alias:"help"}).describe("h","Show this help")
	.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
	.options("d",{alias:"debug"}).describe("d","Activate profiling and debug output")
	.options("host",{default:"localhost"}).describe("host","Server Host")
	.options("port",{default:3101}).describe("port","Server port")
	argv = optimist.argv;
	
// Library path
exports.serverpath = argv.p;

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
