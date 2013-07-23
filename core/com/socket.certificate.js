var configs = require('../config.js');
var fs = require('fs');

exports.readFromCertificatePaths = function(filenames, directories) {
	var full_filenames = [];
	filenames.forEach(function(filename){
		directories.forEach(function(directory){
			full_filenames.push(directory+'/'+filename);
		});
	});
	var targetname = null;
	full_filenames.forEach(function(filename){
		if (fs.existsSync(filename)) {
			targetname = filename;
			return;
		}
	});
	
	if (targetname)
		return fs.readFileSync(targetname,{encoding:'utf8'});
	
	console.error("SSL Key/Cert file not found. Provided names: "+full_filenames);
	process.exit(1);
}

/**
 * Read all allowed CAs
 */
exports.readAllowedCertificates = function(directories) {
	var allowed_ca = new Array();
	directories.forEach(function(dir){
		if (fs.existsSync(dir)) {
			files =  fs.readdirSync(dir).filter(function(file) { return file.substr(-4) == '.crt'; });
			files.forEach(function(file){
				var filedata = fs.readFileSync(dir+'/'+file,{encoding:'utf8'});
				if (filedata==null) return;
				allowed_ca.push(filedata);
			});
		}
	});
	if (allowed_ca.length==0)
		allowed_ca = null;
	return allowed_ca;
}
