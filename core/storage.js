var controlflow = require('async');
var mongo = require('mongoskin');
var fs = require('fs');
var fsFile = require('file');
var configs = require('./config.js');
var db = mongo.db('localhost:27017/'+configs.runtimeconfig.databasename, {w: 1});
// 
// db.collection('blog').find().toArray(function (err, items) {
// 	console.dir(items);
// })
// 
// db.createCollection("bla", function(err, collection){
// 	collection.insert({"test":"value"}, function(err2) {});
// 	console.log("create ok", err);
// 	});

var importpaths = [];

function baseName(str)
{
	var base = new String(str).substring(str.lastIndexOf('/') + 1); 
	if(base.lastIndexOf(".") != -1)       
		base = base.substring(0, base.lastIndexOf("."));
	return base;
}
function dirName(str)
{
	var i = str.lastIndexOf('/')-1; i = str.lastIndexOf('/', i);
	var base = new String(str).substring(i + 1); 
	if(base.lastIndexOf("/") != -1)       
		base = base.substring(0, base.lastIndexOf("/"));
	return base;
}

/****** Verifier *********/
exports.genericverifier = function(doc) {
	this.setIDIfNotExist = function(doc, filename) {
		if (!doc.componentid_)
			doc.componentid_ = dirName(filename);

		if (!doc.id_) {
			if (doc.type_=="configuration")
				doc.id_ = doc.componentid_ + "." + doc.instanceid_;
			else if (doc.type_=="description")
				doc.id_ = doc.componentid_ + "." + doc.language;
			else
				doc.id_ = baseName(filename);
		}
	}
	this.isValid = function(doc) {
		var valid_types = ["event","condition","action","configuration","schema","description"];
		if (doc.type_=="description" && (!doc.language || !doc.name)) {
			return false;
		}
		if (doc.type_=="scene" && (!doc.name)) {
			return false;
		}
		if (doc.type_=="configuration" && !doc.instanceid_) {
			return false;
		}
		return (doc.componentid_ && doc.id_);
	}
};

/****** addImportPath *********/
exports.addImportPath = function(path) {if (path) importpaths.push(path);}

/****** init *********/
exports.init = function(callback) {
	callback(null, null);
}

/****** db *********/
exports.db = db;

/****** helper *********/
exports.getScenes = function(callback) {
	exports.db.collection('scene').find().toArray(callback);
}
exports.getEventsForScene = function(sceneid, callback) {
	exports.db.collection('event').find({sceneid_:sceneid}).toArray(callback);
}
exports.getSceneItem = function(type, id, callback) {
	exports.db.collection(type).find({id_:id}).toArray(callback);
}

/****** importNewFiles *********/
exports.importNewFiles = function(callback) {
	var verifier = new exports.genericverifier();
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
			if (exports.overwrite)
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


exports.showstats = function(callback) {
	console.log("Database:");
	var q = controlflow.queue(function (task, queuecallback) {
		db.collection(task.coll.collectionName).count(function(err, res){
			console.log("\t"+task.coll.collectionName, res);
			queuecallback();
		});
	},1);
	q.drain = function() { callback(null, null); }
	
	db.collections(function(err, collections) {
		if (err)
			return;
		collections.forEach(function(coll) {
			q.push({coll:coll});
		});
	});
}
