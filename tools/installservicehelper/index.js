#!/usr/bin/node
/**/

var optimist = require('optimist').usage('Usage: $0	 [options]')
	.options("h",{alias:"help"}).describe("h","Show this help")
	.options("v",{alias:"version"}).describe("v","Show version and other information as parsable json")
	.options("i",{alias:"import",required:true}).describe("i","Import json files from path")
	.options("e",{alias:"export",required:true}).describe("e","Export processed files to this path")
	.options("s",{alias:"service",required:true}).describe("s","The service id")
	argv = optimist.argv;
	
if (argv.help) {
	console.log("Please visit http://davidgraeff.github.com/scenecontrol for more information");
	console.log(optimist.help());
	process.exit(0);
} else if (argv.version) {
	console.log("1.0");
	process.exit(0);
}