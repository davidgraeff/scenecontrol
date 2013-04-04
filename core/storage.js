var controlflow = require('async');
var mongo = require('mongoskin');
var fs = require('fs');
var fsFile = require('file');
var configs = require('./config.js');
var api = require('./com/api.js');
var storageListener = require('./storage.listener.js');
var storageverifier = require('./storage.verifydoc.js');
var verifier = new storageverifier.genericverifier();
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

exports.update = function(doc, storageCode, onStorageErrorCallback) {
	if (!verifier.isValidForInsert(doc) || !doc.id_) {
		if (onStorageErrorCallback)
			onStorageErrorCallback("storage.update", "Storage update failed. Doc invalid!");
		return;
	}
	
	console.log("UPDATE STORE", doc);
	
	db.collection(doc.type_).update({id_:doc.id_}, doc, { safe:true, upsert: false, strict:true }, function(err) {
		if (err) {
			if (onStorageErrorCallback)
				onStorageErrorCallback("storage.update", "Storage update failed; "+err);
			return;
		}
		storageListener.change(doc, storageCode);
	});
}

/**
 * Inserting a new scene item needs a separate method because the scene
 */
exports.insertSceneItem = function(doc, storageCode, onStorageErrorCallback) {
	if (doc.id_ || !verifier.isValidForInsert(doc)) {
		if (onStorageErrorCallback)
			onStorageErrorCallback("storage.insert", "Storage insert failed. Doc invalid!");
		return;
	}
	
	doc.id_ = require('node-uuid').v1();
	console.log("INSERT SCENEITEM STORE", doc);
	

	exports.db.collection('scene').find({id_:doc.sceneid_}).toArray(function(err, items) {
		if (err || items.length!=1 || !items[0].v) {
			console.warn("Storage insert failed, Scene not found for new scene item", doc, err);
			if (onStorageErrorCallback)
				onStorageErrorCallback("storage.insert", err);
			return;
		}
		var scenedoc = items[0];
		db.collection(doc.type_).insert(doc, { safe:true, strict:true }, function(err) {
			if (err) {
				console.warn("Storage insert failed", doc, err);
				if (onStorageErrorCallback)
					onStorageErrorCallback("storage.insert", err);
				return;
			}
			storageListener.change(doc, storageCode);
			scenedoc.v.push({id_:doc.id_, type_: doc.type_});
			exports.update(scenedoc, storageCode, onStorageErrorCallback);
		});
	});
}

exports.insert = function(doc, storageCode, onStorageErrorCallback) {
	if (doc.id_ || !verifier.isValidForInsert(doc)) {
		if (onStorageErrorCallback)
			onStorageErrorCallback("storage.insert", "Storage insert failed. Doc invalid!");
		return;
	}
	
	doc.id_ = require('node-uuid').v1();
	console.log("INSERT STORE", doc);
	
	db.collection(doc.type_).insert(doc, { safe:true, strict:true }, function(err) {
		if (err) {
			console.warn("Storage update failed", doc, err);
			if (onStorageErrorCallback)
				onStorageErrorCallback("storage.insert", err);
			return;
		}
		storageListener.change(doc, storageCode);
	});
}

exports.remove = function(doc, storageCode, onStorageErrorCallback) {
	if (!doc.id_ || !doc.type_) {
		if (onStorageErrorCallback)
			onStorageErrorCallback("storage.remove", "Storage remove failed. Doc invalid!");
		return;
	}
	
	console.log("REMOVE STORE", doc);
	db.collection(doc.type_).remove({id_:doc.id_}, function(err) {
		if (err) {
			if (onStorageErrorCallback)
				onStorageErrorCallback("storage.remove", err);
			return;
		}
		storageListener.remove(doc, storageCode);
		// if this document referenced to a scene, we have to remove the item from the scene, too.
		if (doc.sceneid_) {
			// fetch scene
			exports.db.collection('scene').find({id_:doc.sceneid_}).toArray(function(err, items) {
				if (err || items.length!=1 || !items[0].v) {
					console.warn("Storage remove failed, Scene not found for removed scene item", doc, err);
					if (onStorageErrorCallback)
						onStorageErrorCallback("storage.remove", err);
					return;
				}
				var scenedoc = items[0];
				// remove nodes and edges belonging to the removed element
				for (var i=scenedoc.v.length-1;i>=0;--i) {
					if (api.uidEqual(scenedoc.v[i], doc)) {
						scenedoc.v.splice(i, 1);
					} else {
						//TODO remove edges
					}
				}
				// update scene
				exports.update(scenedoc, storageCode, onStorageErrorCallback);
			}
		}
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
