var controlflow = require('async');
var fs = require('fs');
var fsFile = require('file');
var storage = require('./storage.js');
var storageverifier = require('./storage.verifydoc.js');
var db = storage.db;
var configs = require('./config.js');

var importpaths = [];

/****** addImportPath *********/
exports.addImportPath = function(path) {if (path) importpaths.push(path);}
exports.overwrite = false;

/****** importNewFiles *********/
exports.importNewFiles = function(callback) {
	var verifier = new storageverifier.genericverifier();
	var remaining = 0, processed = 0;
	var allfiles = [];
	
	importpaths.forEach(function(dirname) {
		fsFile.walkSync(dirname, function(dirPath, dirs, files) {
			if (!files) return;
			var files = files.filter(function(file) {return file.substr(-5) == '.json'; });
			files.forEach(function(file) {
				allfiles.push(dirPath+"/"+file);
			});
		});
	});
	
	// setup queue
	var q = controlflow.queue(function (task, queuecallback) {
		var doc = task.doc;
		var docname = doc.type_ + " "+ doc.id_;
		//console.log('process: ' + docname);
		db.collection(doc.type_).ensureIndex("id_", {unique: true, strict:true}, function(err,name) {
			if (err && err.ok!=1)
				console.error("DB Index error:", err);
			if (storage.overwrite)
				db.collection(doc.type_).update({id_:doc.id_}, doc, { upsert: true, strict:true }, function(err, result) {
					if (err) {
						console.warn("Could not import invalid file ", err);
					}
					if (result) {
						console.log("Imported/Updated file: ", docname);
						++processed;
					}
					--remaining;
					queuecallback();
				});
			else
				db.collection(doc.type_).insert(doc, {strict:true }, function(err, result) {
					if (err && err.code!=11000) {
						console.warn("Could not import invalid file ", err);
					}
					if (result) {
						console.log("Imported file: ", docname);
						++processed;
					}
					--remaining;
					queuecallback();
				});
		});
	}, 2);
	
	// queue processing finished
	q.drain = function() {
// 		console.log('all items have been processed', remaining);
		if (remaining == 0) {
			console.log("Imported files: "+processed);
			callback(null, null);
		}
	}

	// start adding files to queue
	remaining = allfiles.length;
	
	allfiles.forEach(function(jsonfilename) {
		var doc;
		try {
			doc = JSON.parse(fs.readFileSync(jsonfilename, 'utf8'));
		} catch(e) {
			console.warn("Could not import invalid file "+jsonfilename, e);
			--remaining;
			return;
		}
		
		verifier.setIDIfNotExist(doc, jsonfilename);

		if (!verifier.isValid(doc)) {
			console.warn("Verifier denied importing of ", doc);
			--remaining;
			return;
		}
		
		q.push({doc:doc});
	});

	if (remaining == 0)
		callback(null, null);
}
