var controlflow = require('async');
var mongo = require('mongoskin');
var fs = require('fs');
var fsFile = require('file');
var configs = require('./config.js');
var storageListener = require('./storage.listener.js');
var db = mongo.db('localhost:27017/'+configs.runtimeconfig.databasename, {w: 1});

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
exports.update = function(doc) {
	db.collection(doc.type_).update({id_:doc.id_}, doc, { safe:true, upsert: true, strict:true }, function(err) {
		if (err) {
			return;
		}
		storageListener.change(doc);
	});
}
exports.remove = function(doc) {
	db.collection(doc.type_).remove({id_:doc.id_}, function(err) {
		if (err) {
			return;
		}
		storageListener.remove(doc);
	});
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
